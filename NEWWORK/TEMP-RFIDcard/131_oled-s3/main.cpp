#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2c_master.h>
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_ssd1306.h>
#include <WiFiManager.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "ai_vox_engine.h"
#include "audio_device/audio_input_device_i2s_std.h"
#include "audio_device/audio_input_device_pdm.h"
#include "audio_device/audio_output_device_i2s_std.h"
#include "components/espressif/button/button_gpio.h"
#include "components/espressif/button/iot_button.h"
#include "display.h"

#ifndef ARDUINO_ESP32S3_DEV
#error "This example only supports ESP32S3-Dev board."
#endif

#define AUDIO_INPUT_DEVICE_TYPE_I2S_STD (1)
#define AUDIO_INPUT_DEVICE_TYPE AUDIO_INPUT_DEVICE_TYPE_I2S_STD

// ---------------- 引脚定义 ----------------
constexpr gpio_num_t kMicPinSck = GPIO_NUM_5;
constexpr gpio_num_t kMicPinWs  = GPIO_NUM_4;
constexpr gpio_num_t kMicPinSd  = GPIO_NUM_6;

constexpr gpio_num_t kSpeakerPinSck = GPIO_NUM_15;
constexpr gpio_num_t kSpeakerPinWs  = GPIO_NUM_16;
constexpr gpio_num_t kSpeakerPinSd  = GPIO_NUM_7;

constexpr gpio_num_t kI2cPinSda = GPIO_NUM_41;
constexpr gpio_num_t kI2cPinScl = GPIO_NUM_42;

constexpr gpio_num_t kButtonBoot = GPIO_NUM_0;  // BOOT

// RC522
constexpr int kRc522PinSda  = 10;   // SS
constexpr int kRc522PinSck  = 12;
constexpr int kRc522PinMosi = 11;
constexpr int kRc522PinMiso = 13;
constexpr int kRc522PinRst  = 14;

constexpr byte kRfidDataBlock = 1;  // 扇区0的数据块1（非尾块）

// DFPlayer (UART2)
constexpr int kDfPlayerTx   = GPIO_NUM_17;
constexpr int kDfPlayerRx   = GPIO_NUM_18;
constexpr int kDfPlayerBusy = GPIO_NUM_21;

// ---------------- 全局对象 ----------------
std::unique_ptr<Display> g_display;
auto g_observer = std::make_shared<ai_vox::Observer>();
auto g_audio_output_device = std::make_shared<ai_vox::AudioOutputDeviceI2sStd>(
    kSpeakerPinSck, kSpeakerPinWs, kSpeakerPinSd);

button_handle_t g_button_boot_handle = nullptr;
MFRC522 mfrc522(kRc522PinSda, kRc522PinRst);

DFRobotDFPlayerMini g_dfplayer;
HardwareSerial dfplayerSerial(2);

// ---------------- 状态变量 ----------------
bool g_wifi_connected = false;

bool g_rc522_initialized = false;
SemaphoreHandle_t g_rfid_mutex = nullptr;
volatile bool g_writing_in_progress = false;

String g_last_card_info = "";
unsigned long g_last_card_time = 0;

bool g_dfplayer_initialized = false;
int64_t g_current_volume = 20;
bool g_is_playing = false;
bool g_single_play_mode = false;
int64_t g_current_folder = 1;
int64_t g_current_track = 1;

// DFPlayer busy抖动处理
unsigned long g_ignore_busy_until = 0;
const int kBusyDebounceCount = 5;
int g_busy_high_counter = 0;

// ---------------- 函数声明 ----------------
void InitDisplay();
void HandleWiFiConnection();
void InitMcpTools();
bool InitRC522();
bool InitDFPlayer();
void WiFiResetCallback(void* arg, void* data);

bool IsMifareClassicCard();
bool SelectCardSimple(uint32_t timeoutMs);
bool MFRC522_ReadBlock(byte block, byte *buffer, byte *length);
bool MFRC522_WriteBlock(byte block, byte *buffer, byte length);
String ParseCardInfo(byte *data, byte len);
bool WriteGameCard(uint16_t cardNumber, uint32_t gameNumber);
String ReadCurrentCardInfo();                          // 非阻塞（轮询）
String ReadCurrentCardInfoBlocking(uint32_t timeoutMs);// 阻塞（MCP命令）

void PlayTrack(int folder, int track);
void PlayTrack(int track);
void PlayNextTrack();
void PlayPreviousTrack();

// ================== RFID ==================
bool IsMifareClassicCard() {
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    return (piccType == MFRC522::PICC_TYPE_MIFARE_MINI ||
            piccType == MFRC522::PICC_TYPE_MIFARE_1K ||
            piccType == MFRC522::PICC_TYPE_MIFARE_4K);
}

// 简单稳定的选卡流程（主要用于写卡）
bool SelectCardSimple(uint32_t timeoutMs) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
            return true;
        }
        delay(20);
    }
    return false;
}

bool MFRC522_ReadBlock(byte block, byte *buffer, byte *length) {
    if (!buffer || !length) return false;

    byte tmp[18];
    byte tmpSize = sizeof(tmp);

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Auth failed (read block ");
        Serial.print(block);
        Serial.print("): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    status = mfrc522.MIFARE_Read(block, tmp, &tmpSize);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Read failed (block ");
        Serial.print(block);
        Serial.print("): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    memcpy(buffer, tmp, 16);
    *length = 16;
    return true;
}

bool MFRC522_WriteBlock(byte block, byte *buffer, byte length) {
    if (!buffer || length != 16) {
        Serial.println("Write failed: data length must be 16 bytes.");
        return false;
    }

    // 禁止写扇区尾块（密钥块）
    if ((block % 4) == 3) {
        Serial.println("Write failed: trailer block is forbidden.");
        return false;
    }

    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Auth failed (write block ");
        Serial.print(block);
        Serial.print("): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    status = mfrc522.MIFARE_Write(block, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Write failed (block ");
        Serial.print(block);
        Serial.print("): ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    return true;
}

String ParseCardInfo(byte *data, byte len) {
    if (len < 16) return "数据不足";

    uint16_t calc_sum = 0;
    for (int i = 0; i < 7; i++) calc_sum += data[i];
    uint16_t stored_sum = ((uint16_t)data[7] << 8) | data[8];
    if (calc_sum != stored_sum) return "校验错误";

    uint8_t cardType = data[0];
    uint16_t cardNumber = ((uint16_t)data[1] << 8) | data[2];

    if (cardType == 0x01) {
        uint32_t gameNumber = ((uint32_t)data[3] << 24) |
                              ((uint32_t)data[4] << 16) |
                              ((uint32_t)data[5] << 8)  |
                              (uint32_t)data[6];
        char buf[64];
        snprintf(buf, sizeof(buf), "游戏卡 编号:%u 数字:%lu",
                 cardNumber, (unsigned long)gameNumber);
        return String(buf);
    } else if (cardType == 0x02) {
        char buf[32];
        snprintf(buf, sizeof(buf), "任务卡 编号:%u", cardNumber);
        return String(buf);
    }
    return "未知卡类型";
}

bool WriteGameCard(uint16_t cardNumber, uint32_t gameNumber) {
    if (!g_rc522_initialized || !g_rfid_mutex) {
        Serial.println("WriteGameCard: RC522 not ready.");
        return false;
    }

    if (xSemaphoreTake(g_rfid_mutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
        Serial.println("WriteGameCard: RFID mutex timeout.");
        return false;
    }

    g_writing_in_progress = true;
    g_display->ShowStatus("请放卡...");

    // 准备16字节数据
    byte data[16] = {0};
    data[0] = 0x01; // 游戏卡
    data[1] = (cardNumber >> 8) & 0xFF;
    data[2] = cardNumber & 0xFF;
    data[3] = (gameNumber >> 24) & 0xFF;
    data[4] = (gameNumber >> 16) & 0xFF;
    data[5] = (gameNumber >> 8) & 0xFF;
    data[6] = gameNumber & 0xFF;

    uint16_t checksum = 0;
    for (int i = 0; i < 7; i++) checksum += data[i];
    data[7] = (checksum >> 8) & 0xFF;
    data[8] = checksum & 0xFF;

    bool ok = false;
    const int maxTry = 3;

    for (int t = 0; t < maxTry && !ok; ++t) {
        Serial.printf("WriteGameCard: try %d/%d\n", t + 1, maxTry);

        // 每次重试前重置读卡器，清状态
        mfrc522.PCD_Reset();
        delay(5);
        mfrc522.PCD_Init();
        mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
        delay(10);

        if (!SelectCardSimple(2000)) {
            Serial.println("WriteGameCard: no card selected.");
            continue;
        }

        Serial.print("Card UID:");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(mfrc522.uid.uidByte[i], HEX);
        }
        Serial.println();

        if (!IsMifareClassicCard()) {
            Serial.println("WriteGameCard: not MIFARE Classic.");
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            delay(30);
            continue;
        }

        g_display->ShowStatus("写入中...");
        bool writeOK = MFRC522_WriteBlock(kRfidDataBlock, data, 16);
        if (!writeOK) {
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            delay(60);
            continue;
        }

        // 回读校验
        byte verify[16];
        byte vlen = 16;
        bool readOK = MFRC522_ReadBlock(kRfidDataBlock, verify, &vlen);

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();

        if (readOK && memcmp(data, verify, 16) == 0) {
            ok = true;
            break;
        } else {
            Serial.println("WriteGameCard: verify failed.");
            delay(60);
        }
    }

    g_writing_in_progress = false;
    xSemaphoreGive(g_rfid_mutex);

    if (ok) {
        Serial.println("WriteGameCard: SUCCESS");
        g_display->ShowStatus("写入成功");
    } else {
        Serial.println("WriteGameCard: FAILED");
        g_display->ShowStatus("写入失败");
    }
    return ok;
}

// 非阻塞读卡：用于轮询（只读“新进入”卡）
String ReadCurrentCardInfo() {
    if (!g_rc522_initialized || !g_rfid_mutex) return "RC522未初始化";
    if (g_writing_in_progress) return "写卡中";

    if (xSemaphoreTake(g_rfid_mutex, pdMS_TO_TICKS(30)) != pdTRUE) {
        return "RFID忙";
    }

    String ret = "无卡";

    do {
        if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
            ret = "无卡";
            break;
        }

        if (!IsMifareClassicCard()) {
            ret = "非Classic卡";
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            break;
        }

        byte block1[16];
        byte len = 16;
        if (!MFRC522_ReadBlock(kRfidDataBlock, block1, &len)) {
            ret = "读取块1失败";
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            break;
        }

        ret = ParseCardInfo(block1, len);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    } while (0);

    xSemaphoreGive(g_rfid_mutex);
    return ret;
}

// 阻塞读卡：用于 MCP 命令 read_now（可读取“已在场卡”）
String ReadCurrentCardInfoBlocking(uint32_t timeoutMs) {
    if (!g_rc522_initialized || !g_rfid_mutex) return "RC522未初始化";
    if (g_writing_in_progress) return "写卡中";

    if (xSemaphoreTake(g_rfid_mutex, pdMS_TO_TICKS(timeoutMs + 200)) != pdTRUE) {
        return "RFID忙";
    }

    String ret = "无卡";
    unsigned long start = millis();

    while (millis() - start < timeoutMs) {
        bool selected = false;

        // 路径1：新卡检测
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
            selected = true;
        } else {
            // 路径2：唤醒已在场卡（关键）
            byte atqa[2];
            byte atqaSize = sizeof(atqa);
            MFRC522::StatusCode s = mfrc522.PICC_WakeupA(atqa, &atqaSize);
            if (s == MFRC522::STATUS_OK) {
                if (mfrc522.PICC_ReadCardSerial()) {
                    selected = true;
                }
            }
        }

        if (!selected) {
            delay(30);
            continue;
        }

        if (!IsMifareClassicCard()) {
            ret = "非Classic卡";
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            break;
        }

        byte block1[16];
        byte len = 16;
        if (!MFRC522_ReadBlock(kRfidDataBlock, block1, &len)) {
            ret = "读取块1失败";
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
            break;
        }

        ret = ParseCardInfo(block1, len);
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        break;
    }

    xSemaphoreGive(g_rfid_mutex);
    return ret;
}

bool InitRC522() {
    Serial.println("Initializing RC522...");
    SPI.begin(kRc522PinSck, kRc522PinMiso, kRc522PinMosi, kRc522PinSda);
    SPI.setFrequency(4000000);  // 关键：集成环境更稳

    mfrc522.PCD_Init();
    delay(20);

    byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    Serial.printf("RC522 Version: 0x%02X\n", v);
    if (v == 0x00 || v == 0xFF) return false;

    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

    if (!g_rfid_mutex) g_rfid_mutex = xSemaphoreCreateMutex();
    if (!g_rfid_mutex) return false;

    g_rc522_initialized = true;
    return true;
}

// ================== DFPlayer ==================
bool InitDFPlayer() {
    Serial.println("Initializing DFPlayer Mini...");
    Serial.println("NOTE: MP3 files should be /01/001.mp3 format.");

    dfplayerSerial.begin(9600, SERIAL_8N1, kDfPlayerRx, kDfPlayerTx);
    delay(500);

    if (!g_dfplayer.begin(dfplayerSerial, false, true)) {
        Serial.println("DFPlayer initialization FAILED!");
        g_dfplayer_initialized = false;
        return false;
    }

    delay(800);
    g_dfplayer.volume((int)g_current_volume);
    g_dfplayer.EQ(DFPLAYER_EQ_NORMAL);
    g_dfplayer.outputDevice(DFPLAYER_DEVICE_SD);

    if (kDfPlayerBusy > 0) pinMode(kDfPlayerBusy, INPUT);

    g_dfplayer_initialized = true;
    Serial.printf("DFPlayer initialized, volume=%lld\n", g_current_volume);
    return true;
}

void PlayTrack(int folder, int track) {
    if (!g_dfplayer_initialized) return;

    if (folder < 1 || folder > 99) folder = 1;
    if (track < 1 || track > 255) track = 1;

    g_dfplayer.playFolder(folder, track);
    g_current_folder = folder;
    g_current_track = track;
    g_is_playing = true;

    g_ignore_busy_until = millis() + 1000;
    g_busy_high_counter = 0;

    String s = "播放: " + String(folder) + "-" + String(track);
    g_display->ShowStatus(s.c_str());
}
void PlayTrack(int track) { PlayTrack(1, track); }
void PlayNextTrack() {
    int t = (int)g_current_track + 1;
    if (t > 255) t = 1;
    PlayTrack((int)g_current_folder, t);
}
void PlayPreviousTrack() {
    int t = (int)g_current_track - 1;
    if (t < 1) t = 255;
    PlayTrack((int)g_current_folder, t);
}

// ================== 显示 / WiFi / MCP ==================
void InitDisplay() {
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = kI2cPinSda,
        .scl_io_num = kI2cPinScl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_i2c_config_t io_cfg = {
        .dev_addr = 0x3C,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .scl_speed_hz = 400000
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(bus, &io_cfg, &io));

    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1
    };
    esp_lcd_panel_ssd1306_config_t ssd1306_cfg = { .height = 64 };
    panel_cfg.vendor_config = &ssd1306_cfg;

    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io, &panel_cfg, &panel));
    esp_lcd_panel_reset(panel);
    esp_lcd_panel_init(panel);
    esp_lcd_panel_disp_on_off(panel, true);

    g_display = std::make_unique<Display>(io, panel, 128, 64, true, true);
    g_display->Start();
}

void InitMcpTools() {
    auto& engine = ai_vox::Engine::GetInstance();

    engine.AddMcpTool("self.rfid.get_last_card", "Get last RFID card info", {});
    engine.AddMcpTool("self.rfid.read_now", "Read card info now", {});
    engine.AddMcpTool("self.rfid.write_game_card", "Write game card", {
        {"number", ai_vox::ParamSchema<int64_t>{}}
    });

    engine.AddMcpTool("self.dfplayer.play", "Play MP3 from folder", {
        {"folder", ai_vox::ParamSchema<int64_t>{}},
        {"track", ai_vox::ParamSchema<int64_t>{}}
    });
    engine.AddMcpTool("self.dfplayer.pause", "Pause", {});
    engine.AddMcpTool("self.dfplayer.resume", "Resume", {});
    engine.AddMcpTool("self.dfplayer.stop", "Stop", {});
    engine.AddMcpTool("self.dfplayer.volume", "Set volume", {
        {"level", ai_vox::ParamSchema<int64_t>{}}
    });
    engine.AddMcpTool("self.dfplayer.next", "Next", {});
    engine.AddMcpTool("self.dfplayer.previous", "Previous", {});
    engine.AddMcpTool("self.dfplayer.get_status", "Get status", {});
}

void WiFiResetCallback(void* arg, void* data) {
    g_display->ShowStatus("正在重置WiFi...");
    delay(1000);

    WiFiManager wm;
    wm.resetSettings();

    g_display->ShowStatus("WiFi已清除\n系统重启中");
    delay(2000);
    ESP.restart();
}

void HandleWiFiConnection() {
    static unsigned long press_start_time = 0;
    static bool is_pressing = false;

    bool current_pressed = (digitalRead(kButtonBoot) == LOW);

    if (current_pressed) {
        if (!is_pressing) {
            is_pressing = true;
            press_start_time = millis();
        } else {
            unsigned long duration = millis() - press_start_time;
            static unsigned long last_ui = 0;
            if (millis() - last_ui > 200) {
                last_ui = millis();
                int remaining = 5 - (duration / 1000);
                if (remaining < 0) remaining = 0;
                String s = "配网倒计时: " + String(remaining);
                g_display->ShowStatus(s.c_str());
            }

            if (duration > 5000) {
                g_display->ShowStatus("正在开启热点...");
                WiFiManager wm;
                wm.setConfigPortalTimeout(180);
                wm.setBreakAfterConfig(true);
                bool res = wm.startConfigPortal("AI-燕归乐");

                if (res) {
                    g_display->ShowStatus("WiFi 配网成功!");
                    g_wifi_connected = true;
                    delay(1000);
                } else {
                    g_display->ShowStatus("配网失败\n系统将重启");
                    delay(2000);
                    ESP.restart();
                }
                is_pressing = false;
            }
        }
    } else if (is_pressing) {
        is_pressing = false;
        g_display->ShowStatus("长按5秒配网");
    }
}

// ================== Arduino ==================
void setup() {
    Serial.begin(115200);
    pinMode(kButtonBoot, INPUT_PULLUP);

    InitDisplay();
    g_display->ShowStatus("系统启动中...");

    if (InitRC522()) {
        Serial.println("RC522 Ready");
    } else {
        g_display->ShowStatus("RC522 错误");
    }

    if (InitDFPlayer()) {
        g_single_play_mode = true;
        PlayTrack(1, 99);  // 欢迎音
    } else {
        g_display->ShowStatus("DFPlayer 错误");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin();

    unsigned long start = millis();
    g_display->ShowStatus("正在连接WiFi...");
    while (millis() - start < 3000) {
        if (WiFi.status() == WL_CONNECTED) {
            g_wifi_connected = true;
            g_display->ShowStatus("WiFi 已连接");
            break;
        }
        delay(100);
    }
    if (!g_wifi_connected) g_display->ShowStatus("连接失败\n长按5秒配网");
}

void loop() {
    if (!g_wifi_connected) {
        HandleWiFiConnection();
        delay(50);
        return;
    }

    static bool engine_started = false;
    if (!engine_started) {
        engine_started = true;
        InitMcpTools();

        auto audio_input_device = std::make_shared<ai_vox::AudioInputDeviceI2sStd>(
            kMicPinSck, kMicPinWs, kMicPinSd);

        auto& engine = ai_vox::Engine::GetInstance();
        engine.SetObserver(g_observer);
        engine.SetOtaUrl("https://api.tenclass.net/xiaozhi/ota/");
        engine.ConfigWebsocket(
            "wss://api.tenclass.net/xiaozhi/v1/",
            {{"Authorization", "Bearer test-token"}});
        engine.Start(audio_input_device, g_audio_output_device);

        g_display->ShowStatus("AI 已就绪");

        const button_config_t btn_cfg = {
            .long_press_time = 5000,
            .short_press_time = 50
        };
        const button_gpio_config_t gpio_cfg = {
            .gpio_num = kButtonBoot,
            .active_level = 0
        };
        iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &g_button_boot_handle);

        iot_button_register_cb(g_button_boot_handle, BUTTON_PRESS_DOWN, nullptr,
            [](void* b, void* u) {
                ai_vox::Engine::GetInstance().Advance();
            }, nullptr);

        iot_button_register_cb(g_button_boot_handle, BUTTON_LONG_PRESS_START, nullptr,
            WiFiResetCallback, nullptr);
    }

    // 周期刷卡（写卡期间严格不读）
    static unsigned long last_rfid_check = 0;
    if (!g_writing_in_progress && millis() - last_rfid_check > 500) {
        last_rfid_check = millis();

        if (g_rc522_initialized) {
            String info = ReadCurrentCardInfo();
            if (info != "无卡" &&
                info != "读取块1失败" &&
                info != "RC522未初始化" &&
                info != "非Classic卡" &&
                info != "RFID忙" &&
                info != "写卡中") {
                g_last_card_info = info;
                g_last_card_time = millis();
                g_display->ShowStatus(info.c_str());

                g_single_play_mode = true;
                PlayTrack(1, 98);
            }
        }
    }

    // DFPlayer 状态维护
    if (g_dfplayer_initialized && g_is_playing) {
        while (g_dfplayer.available()) {
            int type = g_dfplayer.readType();
            int value = g_dfplayer.read();
            if (type == DFPlayerError) {
                Serial.printf("DFPlayer error: %d\n", value);
            }
        }

        if (millis() > g_ignore_busy_until) {
            bool busy_pin = digitalRead(kDfPlayerBusy);
            if (busy_pin == HIGH) {
                g_busy_high_counter++;
                if (g_busy_high_counter >= kBusyDebounceCount) {
                    g_busy_high_counter = 0;
                    if (g_single_play_mode) {
                        g_dfplayer.stop();
                        g_is_playing = false;
                        g_single_play_mode = false;
                        g_display->ShowStatus("播放结束");
                    } else {
                        PlayNextTrack();
                    }
                }
            } else {
                g_busy_high_counter = 0;
            }
        }
    } else {
        g_busy_high_counter = 0;
    }

    // AI 事件处理
    auto& engine = ai_vox::Engine::GetInstance();
    const auto events = g_observer->PopEvents();

    for (auto& event : events) {
        if (auto state_event = std::get_if<ai_vox::StateChangedEvent>(&event)) {
            if (state_event->new_state == ai_vox::ChatState::kSpeaking) {
                g_display->ShowStatus("说话中");
            } else if (state_event->new_state == ai_vox::ChatState::kListening) {
                g_display->ShowStatus("聆听中");
            }
        } else if (auto msg_event = std::get_if<ai_vox::ChatMessageEvent>(&event)) {
            g_display->SetChatMessage(msg_event->content);
        } else if (auto mcp_event = std::get_if<ai_vox::McpToolCallEvent>(&event)) {

            if (mcp_event->name == "self.rfid.get_last_card") {
                engine.SendMcpCallResponse(
                    mcp_event->id,
                    g_last_card_info.length() ? g_last_card_info.c_str() : "No card detected");
            }
            else if (mcp_event->name == "self.rfid.read_now") {
                String info = ReadCurrentCardInfoBlocking(2000); // 关键：阻塞读2秒
                if (info == "无卡" && g_last_card_info.length()) {
                    info = "当前未检测到新卡，最近卡信息: " + g_last_card_info;
                }
                engine.SendMcpCallResponse(mcp_event->id, info.c_str());
            }
            else if (mcp_event->name == "self.rfid.write_game_card") {
                auto number = mcp_event->param<int64_t>("number");
                if (!number) {
                    engine.SendMcpCallResponse(mcp_event->id, "Missing number");
                    continue;
                }

                int64_t gameNum = *number;
                if (gameNum < 1 || gameNum > 10000) {
                    engine.SendMcpCallResponse(mcp_event->id, "Number must be 1-10000");
                    continue;
                }

                bool ok = WriteGameCard((uint16_t)gameNum, (uint32_t)gameNum);
                engine.SendMcpCallResponse(
                    mcp_event->id,
                    ok ? "Game card written successfully" : "Write failed");
            }

            else if (mcp_event->name == "self.dfplayer.play") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                auto folder = mcp_event->param<int64_t>("folder");
                auto track  = mcp_event->param<int64_t>("track");

                if (folder && track) {
                    g_single_play_mode = false;
                    PlayTrack((int)*folder, (int)*track);
                    engine.SendMcpCallResponse(mcp_event->id, true);
                } else if (track) {
                    g_single_play_mode = false;
                    PlayTrack((int)*track);
                    engine.SendMcpCallResponse(mcp_event->id, true);
                } else {
                    engine.SendMcpCallResponse(mcp_event->id, false);
                }
            }
            else if (mcp_event->name == "self.dfplayer.pause") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                g_dfplayer.pause();
                g_is_playing = false;
                g_display->ShowStatus("暂停");
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.resume") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                g_dfplayer.start();
                g_is_playing = true;
                g_ignore_busy_until = millis() + 500;
                g_display->ShowStatus("继续播放");
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.stop") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                g_dfplayer.stop();
                g_is_playing = false;
                g_single_play_mode = false;
                g_display->ShowStatus("停止");
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.volume") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                auto volume = mcp_event->param<int64_t>("level");
                if (volume) {
                    int vol = constrain((int)*volume, 0, 30);
                    g_dfplayer.volume(vol);
                    g_current_volume = vol;
                    String s = "音量: " + String(vol);
                    g_display->ShowStatus(s.c_str());
                }
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.next") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                g_single_play_mode = false;
                PlayNextTrack();
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.previous") {
                if (!g_dfplayer_initialized) {
                    engine.SendMcpCallResponse(mcp_event->id, "DFPlayer not initialized");
                    continue;
                }
                g_single_play_mode = false;
                PlayPreviousTrack();
                engine.SendMcpCallResponse(mcp_event->id, true);
            }
            else if (mcp_event->name == "self.dfplayer.get_status") {
                String status = "DFPlayer: ";
                status += g_dfplayer_initialized ? "OK" : "Not initialized";
                status += ", Folder:" + String((int)g_current_folder);
                status += ", Track:" + String((int)g_current_track);
                status += ", ";
                status += g_is_playing ? "Playing" : "Stopped";
                status += ", Vol:" + String((int)g_current_volume);

                engine.SendMcpCallResponse(mcp_event->id, status.c_str());
            }
        }
    }

    delay(10);
}