#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2c_master.h>
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_ssd1306.h>
#include <WiFiManager.h>

#include "ai_vox_engine.h"
#include "audio_device/audio_input_device_i2s_std.h"
#include "audio_device/audio_input_device_pdm.h"
#include "audio_device/audio_output_device_i2s_std.h"
#include "components/espressif/button/button_gpio.h"
#include "components/espressif/button/iot_button.h"
#include "components/espressif/esp_audio_codec/esp_audio_simple_dec.h"
#include "components/espressif/esp_audio_codec/esp_mp3_dec.h"
#include "display.h"

#ifndef ARDUINO_ESP32S3_DEV
#error "This example only supports ESP32S3-Dev board."
#endif

#define AUDIO_INPUT_DEVICE_TYPE_PDM (0)
#define AUDIO_INPUT_DEVICE_TYPE_I2S_STD (1)
#define AUDIO_INPUT_DEVICE_TYPE AUDIO_INPUT_DEVICE_TYPE_I2S_STD
// #define AUDIO_INPUT_DEVICE_TYPE AUDIO_INPUT_DEVICE_TYPE_PDM

// 配网状态机
enum class WiFiConfigState {
  IDLE,
  CHECKING_BUTTON,
  CONFIG_PORTAL,
  CONNECTED
};

namespace {
// Microphone pin configurations
#if AUDIO_INPUT_DEVICE_TYPE == AUDIO_INPUT_DEVICE_TYPE_I2S_STD
constexpr gpio_num_t kMicPinSck = GPIO_NUM_5;  // SCK (BCK, BCLK): Serial-Data Clock for I²S Interface
constexpr gpio_num_t kMicPinWs = GPIO_NUM_4;   // WS (WR, WCLK): Serial Data-Word Select for I²S Interface
constexpr gpio_num_t kMicPinSd = GPIO_NUM_6;   // SD (DIN，DOUT, DI, DO, DATA): Serial-Data Output for I²S Interface
#elif AUDIO_INPUT_DEVICE_TYPE == AUDIO_INPUT_DEVICE_TYPE_PDM
constexpr gpio_num_t kMicPinSck = GPIO_NUM_15;  // SCK (BCK, BCLK): Serial-Data Clock for I²S Interface
constexpr gpio_num_t kMicPinSd = GPIO_NUM_7;    // SD (DIN，DOUT, DI, DO, DATA): Serial-Data Output for I²S Interface
#endif

constexpr gpio_num_t kSpeakerPinSck = GPIO_NUM_15;  // SCK (BCK, BCLK): Serial-Data Clock for I²S Interface
constexpr gpio_num_t kSpeakerPinWs = GPIO_NUM_16;  // WS (WR, WCLK，LRC): Serial Data-Word Select for I²S Interface
constexpr gpio_num_t kSpeakerPinSd = GPIO_NUM_7;  // SD (DIN，DOUT, DI, DO, DATA): Serial-Data Output for I²S Interface

constexpr gpio_num_t kButtonBoot = GPIO_NUM_0;

constexpr gpio_num_t kI2cPinSda = GPIO_NUM_14;
constexpr gpio_num_t kI2cPinScl = GPIO_NUM_13;

constexpr gpio_num_t kLedPin = GPIO_NUM_2;

constexpr uint32_t kDisplayWidth = 128;
constexpr uint32_t kDisplayHeight = 64;
constexpr bool kDisplayMirrorX = true;
constexpr bool kDisplayMirrorY = true;

std::unique_ptr<Display> g_display;
auto g_observer = std::make_shared<ai_vox::Observer>();
auto g_audio_output_device = std::make_shared<ai_vox::AudioOutputDeviceI2sStd>(kSpeakerPinSck, kSpeakerPinWs, kSpeakerPinSd);
button_handle_t g_button_boot_handle = nullptr;

// 配网相关变量
WiFiConfigState g_wifi_config_state = WiFiConfigState::IDLE;
unsigned long g_config_start_time = 0;
constexpr unsigned long kConfigCheckInterval = 50;
constexpr unsigned long kConfigButtonPressTime = 5000; // 5秒长按
constexpr unsigned long kConfigPortalTimeout = 180000; // 3分钟超时
WiFiManager *g_wifiManager = nullptr; // 全局WiFiManager实例

void InitDisplay() {
  printf("InitDisplay\n");
  i2c_master_bus_handle_t display_i2c_bus;
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = kI2cPinSda,
      .scl_io_num = kI2cPinScl,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags =
          {
              .enable_internal_pullup = 1,
              .allow_pd = false,
          },
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus));

  esp_lcd_panel_io_handle_t panel_io = nullptr;
  esp_lcd_panel_io_i2c_config_t io_config = {
      .dev_addr = 0x3C,
      .on_color_trans_done = nullptr,
      .user_ctx = nullptr,
      .control_phase_bytes = 1,
      .dc_bit_offset = 6,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .flags =
          {
              .dc_low_on_data = 0,
              .disable_control_phase = 0,
          },
      .scl_speed_hz = 400 * 1000,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(display_i2c_bus, &io_config, &panel_io));

  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = -1;
  panel_config.bits_per_pixel = 1;

  esp_lcd_panel_ssd1306_config_t ssd1306_config = {
      .height = static_cast<uint8_t>(kDisplayHeight),
  };
  panel_config.vendor_config = &ssd1306_config;

  ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io, &panel_config, &panel));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
  g_display = std::make_unique<Display>(panel_io, panel, kDisplayWidth, kDisplayHeight, kDisplayMirrorX, kDisplayMirrorY);
  g_display->Start();
}

#ifdef PRINT_HEAP_INFO_INTERVAL
void PrintMemInfo() {
  if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0) {
    const auto total_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    const auto free_size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    const auto min_free_size = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    printf("SPIRAM total size: %zu B (%zu KB), free size: %zu B (%zu KB), minimum free size: %zu B (%zu KB)\n",
           total_size,
           total_size >> 10,
           free_size,
           free_size >> 10,
           min_free_size,
           min_free_size >> 10);
  }

  if (heap_caps_get_total_size(MALLOC_CAP_INTERNAL) > 0) {
    const auto total_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    const auto free_size = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    const auto min_free_size = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    printf("IRAM total size: %zu B (%zu KB), free size: %zu B (%zu KB), minimum free size: %zu B (%zu KB)\n",
           total_size,
           total_size >> 10,
           free_size,
           free_size >> 10,
           min_free_size,
           min_free_size >> 10);
  }

  if (heap_caps_get_total_size(MALLOC_CAP_DEFAULT) > 0) {
    const auto total_size = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    const auto free_size = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    const auto min_free_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    printf("DRAM total size: %zu B (%zu KB), free size: %zu B (%zu KB), minimum free size: %zu B (%zu KB)\n",
           total_size,
           total_size >> 10,
           free_size,
           free_size >> 10,
           min_free_size,
           min_free_size >> 10);
  }
}
#endif

// 非阻塞配网处理
void ProcessWiFiConfig() {
  switch (g_wifi_config_state) {
    case WiFiConfigState::IDLE:
      // 检查按钮是否按下
      if (digitalRead(kButtonBoot) == LOW) { // 按钮按下
        g_config_start_time = millis();
        g_wifi_config_state = WiFiConfigState::CHECKING_BUTTON;
        g_display->ShowStatus("请保持按下5秒");
        printf("按键按下，检查保持时间\n");
      }
      break;
      
    case WiFiConfigState::CHECKING_BUTTON:
      // 检查按钮是否释放
      if (digitalRead(kButtonBoot) == HIGH) { // 按钮释放
        g_wifi_config_state = WiFiConfigState::IDLE;
        g_display->ShowStatus("配网取消");
        printf("按键释放，取消配网\n");
        return;
      }
      
      // 检查是否长按5秒
      if (millis() - g_config_start_time > kConfigButtonPressTime) {
        g_wifi_config_state = WiFiConfigState::CONFIG_PORTAL;
        g_display->ShowStatus("进入配网模式");
        printf("进入配网模式\n");
        
        // 启动配置门户
        if (g_wifiManager) {
          delete g_wifiManager;
          g_wifiManager = nullptr;
        }
        g_wifiManager = new WiFiManager();
        
        // 重要：设置AP回调，确保AP启动成功
        g_wifiManager->setAPCallback([](WiFiManager *wm) {
          Serial.println("进入配网模式");
          Serial.print("AP SSID: ");
          Serial.println(wm->getConfigPortalSSID());
          Serial.print("AP IP: ");
          Serial.println(WiFi.softAPIP());
        });
        
        // 关键：设置不自动连接，强制进入配网模式
        g_wifiManager->setConnectTimeout(0); // 不尝试连接
        g_wifiManager->setConfigPortalTimeout(300); // 5分钟超时
        g_wifiManager->setConfigPortalBlocking(false);
        
        // 启动配置门户
        if (!g_wifiManager->startConfigPortal("AI_VOX_Config")) {
          Serial.println("启动配网门户失败");
        } else {
          Serial.println("配网门户已启动");
        }
        g_config_start_time = millis(); // 重置计时器用于超时检测
      }
      break;
      
    case WiFiConfigState::CONFIG_PORTAL:
      if (g_wifiManager) {
        // 处理配置门户
        g_wifiManager->process();
        
        // 检查是否连接成功
        if (WiFi.status() == WL_CONNECTED) {
          g_wifi_config_state = WiFiConfigState::CONNECTED;
          g_display->ShowStatus("WiFi已连接");
          printf("WiFi已连接. IP: %s\n", WiFi.localIP().toString().c_str());
          
          // 清理WiFiManager
          delete g_wifiManager;
          g_wifiManager = nullptr;
          return;
        }
        
        // 检查超时
        if (millis() - g_config_start_time > kConfigPortalTimeout) {
          g_display->ShowStatus("配网超时");
          printf("配网超时\n");
          delay(2000);
          
          // 清理WiFiManager
          delete g_wifiManager;
          g_wifiManager = nullptr;
          
          ESP.restart();
        }
      }
      break;
      
    default:
      break;
  }
}

void StartWiFiConfig() {
  // 尝试连接之前保存的WiFi
  printf("尝试自动连接已保存的WiFi\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  unsigned long start = millis();
  bool connected = false;
  
  while (millis() - start < 5000) { // 尝试连接5秒
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    delay(500);
    Serial.print(".");
  }
  
  if (connected) {
    g_wifi_config_state = WiFiConfigState::CONNECTED;
    g_display->ShowStatus("WiFi已连接");
    printf("自动连接WiFi成功\n");
    return;
  }
  
  // 自动连接失败，进入IDLE状态等待用户按键
  g_wifi_config_state = WiFiConfigState::IDLE;
  printf("自动连接失败，等待按键配网\n");
  g_display->ShowStatus("请长按按键5秒配网");
}

void InitMcpTools() {
  auto& engine = ai_vox::Engine::GetInstance();
  engine.AddMcpTool("self.audio_speaker.set_volume",         // tool name
                    "Set the volume of the audio speaker.",  // tool description
                    {
                        {
                            "volume",  // parameter name

                            ai_vox::ParamSchema<int64_t>{
                                // parameter type can be bool, std::string or int64_t
                                .default_value = std::nullopt,  // default value, set to std::nullopt if not specified
                                .min = 0,                       // minimum value, set to std::nullopt if not specified
                                .max = 100,                     // maximum value, set to std::nullopt if not specified
                            },
                        },
                        // add more parameter schema as needed
                    }  // parameter schema
  );

  engine.AddMcpTool("self.audio_speaker.get_volume",         // tool name
                    "Get the volume of the audio speaker.",  // tool description
                    {
                        // empty
                    }  // parameter schema
  );

  engine.AddMcpTool("self.led.set",                                           // tool name
                    "Set the state of the LED, true for on, false for off.",  // tool description
                    {
                        {
                            "state",  // parameter name
                            ai_vox::ParamSchema<bool>{
                                // parameter type can be bool, std::string or int64_t
                                .default_value = std::nullopt,  // default value, set to std::nullopt if not specified
                            },                                  // parameter type
                        },
                        // add more parameter schema as needed
                    }  // parameter schema
  );

  engine.AddMcpTool("self.led.get",                                           // tool name
                    "Get the state of the LED, true for on, false for off.",  // tool description
                    {
                        // empty
                    }  // parameter schema
  );
}
}  // namespace

void setup() {
  Serial.begin(115200);
  printf("setup\n");

  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, LOW);

  printf("init button\n");
  const button_config_t btn_cfg = {
      .long_press_time = 1000,
      .short_press_time = 50,
  };

  const button_gpio_config_t gpio_cfg = {
      .gpio_num = kButtonBoot,
      .active_level = 0,
      .enable_power_save = false,
      .disable_pull = false,
  };

  ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &g_button_boot_handle));

  InitDisplay();
  g_display->ShowStatus("初始化");
  
  // 启动配网流程
  StartWiFiConfig();
}

void loop() {
  // 处理配网状态机
  if (g_wifi_config_state != WiFiConfigState::CONNECTED) {
    ProcessWiFiConfig();
    delay(kConfigCheckInterval);
    return; // 配网过程中跳过其他处理
  }
  
  // 配网完成后一次性初始化（确保只执行一次）
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    
    InitMcpTools();

#if AUDIO_INPUT_DEVICE_TYPE == AUDIO_INPUT_DEVICE_TYPE_I2S_STD
    auto audio_input_device = std::make_shared<ai_vox::AudioInputDeviceI2sStd>(kMicPinSck, kMicPinWs, kMicPinSd);
#elif AUDIO_INPUT_DEVICE_TYPE == AUDIO_INPUT_DEVICE_TYPE_PDM
    auto audio_input_device = std::make_shared<ai_vox::PdmAudioInputDevice>(kMicPinSck, kMicPinSd);
#endif

    auto& ai_vox_engine = ai_vox::Engine::GetInstance();
    ai_vox_engine.SetObserver(g_observer);
    ai_vox_engine.SetOtaUrl("https://api.tenclass.net/xiaozhi/ota/");
    ai_vox_engine.ConfigWebsocket("wss://api.tenclass.net/xiaozhi/v1/",
                                  {
                                      {"Authorization", "Bearer test-token"},
                                  });
    printf("engine starting\n");
    g_display->ShowStatus("AI引擎启动中");

    ai_vox_engine.Start(audio_input_device, g_audio_output_device);

    printf("engine started\n");

    ESP_ERROR_CHECK(iot_button_register_cb(
        g_button_boot_handle,
        BUTTON_PRESS_DOWN,
        nullptr,
        [](void* button_handle, void* usr_data) {
          printf("boot button pressed\n");
          ai_vox::Engine::GetInstance().Advance();
        },
        nullptr));

    g_display->ShowStatus("AI引擎已启动");
  }

  // 添加强制配网按钮检测
  static unsigned long last_button_check = 0;
  static bool button_pressed = false;
  static unsigned long button_press_start = 0;
  
  if (millis() - last_button_check > 100) {
    last_button_check = millis();
    
    if (digitalRead(kButtonBoot) == LOW) { // 按钮按下（低电平）
      if (!button_pressed) {
        button_pressed = true;
        button_press_start = millis();
        Serial.println("按键按下，开始计时");
      } else {
        // 检查是否长按5秒
        if (millis() - button_press_start > kConfigButtonPressTime) {
          Serial.println("检测到长按，重置WiFi设置");
          g_display->ShowStatus("重置WiFi...");
          
          // 重置WiFi设置
          WiFiManager wifiManager;
          wifiManager.resetSettings();
          WiFi.disconnect(true);
          
          // 重启设备
          delay(1000);
          ESP.restart();
        }
      }
    } else {
      if (button_pressed) {
        button_pressed = false;
        Serial.println("按键释放");
      }
    }
  }

#ifdef PRINT_HEAP_INFO_INTERVAL
  static uint32_t s_print_heap_info_time = 0;
  if (s_print_heap_info_time == 0 || millis() - s_print_heap_info_time >= PRINT_HEAP_INFO_INTERVAL) {
    s_print_heap_info_time = millis();
    PrintMemInfo();
  }
#endif

  auto& engine = ai_vox::Engine::GetInstance();

  const auto events = g_observer->PopEvents();

  for (auto& event : events) {
    if (auto text_received_event = std::get_if<ai_vox::TextReceivedEvent>(&event)) {
      printf("on text received: %s\n", text_received_event->content.c_str());
    } else if (auto activation_event = std::get_if<ai_vox::ActivationEvent>(&event)) {
      printf("activation code: %s, message: %s\n", activation_event->code.c_str(), activation_event->message.c_str());
      g_display->ShowStatus("激活设备");
      g_display->SetChatMessage(activation_event->message);
    } else if (auto state_changed_event = std::get_if<ai_vox::StateChangedEvent>(&event)) {
      switch (state_changed_event->new_state) {
        case ai_vox::ChatState::kIdle: {
          printf("Idle\n");
          break;
        }
        case ai_vox::ChatState::kInitted: {
          printf("Initted\n");
          g_display->ShowStatus("初始化完成");
          break;
        }
        case ai_vox::ChatState::kLoading: {
          printf("Loading...\n");
          g_display->ShowStatus("加载协议中");
          break;
        }
        case ai_vox::ChatState::kLoadingFailed: {
          printf("Loading failed, please retry\n");
          g_display->ShowStatus("加载协议失败，请重试");
          break;
        }
        case ai_vox::ChatState::kStandby: {
          printf("Standby\n");
          g_display->ShowStatus("待命");
          break;
        }
        case ai_vox::ChatState::kConnecting: {
          printf("Connecting...\n");
          g_display->ShowStatus("连接中...");
          break;
        }
        case ai_vox::ChatState::kListening: {
          printf("Listening...\n");
          g_display->ShowStatus("聆听中");
          break;
        }
        case ai_vox::ChatState::kSpeaking: {
          printf("Speaking...\n");
          g_display->ShowStatus("说话中");
          break;
        }
        default: {
          break;
        }
      }
    } else if (auto emotion_event = std::get_if<ai_vox::EmotionEvent>(&event)) {
      printf("emotion: %s\n", emotion_event->emotion.c_str());
      g_display->SetEmotion(emotion_event->emotion);
    } else if (auto chat_message_event = std::get_if<ai_vox::ChatMessageEvent>(&event)) {
      switch (chat_message_event->role) {
        case ai_vox::ChatRole::kAssistant: {
          printf("role: assistant, content: %s\n", chat_message_event->content.c_str());
          g_display->SetChatMessage(chat_message_event->content);
          break;
        }
        case ai_vox::ChatRole::kUser: {
          printf("role: user, content: %s\n", chat_message_event->content.c_str());
          g_display->SetChatMessage(chat_message_event->content);
          break;
        }
      }
    } else if (auto mcp_tool_call_event = std::get_if<ai_vox::McpToolCallEvent>(&event)) {
      printf("on mcp tool call: %s\n", mcp_tool_call_event->ToString().c_str());

      if ("self.audio_speaker.set_volume" == mcp_tool_call_event->name) {
        const auto volume_ptr = mcp_tool_call_event->param<int64_t>("volume");
        if (volume_ptr != nullptr) {
          printf("on mcp tool call: self.audio_speaker.set_volume, volume: %" PRId64 "\n", *volume_ptr);
          g_audio_output_device->set_volume(*volume_ptr);
          engine.SendMcpCallResponse(mcp_tool_call_event->id, true);
        } else {
          engine.SendMcpCallError(mcp_tool_call_event->id, "Missing valid argument: volume");
        }
      } else if ("self.audio_speaker.get_volume" == mcp_tool_call_event->name) {
        const auto volume = g_audio_output_device->volume();
        printf("on mcp tool call: self.audio_speaker.get_volume, volume: %" PRIu16 "\n", volume);
        engine.SendMcpCallResponse(mcp_tool_call_event->id, volume);
      } else if ("self.led.set" == mcp_tool_call_event->name) {
        const auto state_ptr = mcp_tool_call_event->param<bool>("state");
        if (state_ptr != nullptr) {
          printf("on mcp tool call: self.led.set, state: %d\n", *state_ptr);
          digitalWrite(kLedPin, *state_ptr);
          engine.SendMcpCallResponse(mcp_tool_call_event->id, true);
        } else {
          engine.SendMcpCallError(mcp_tool_call_event->id, "Missing valid argument: state");
        }
      } else if ("self.led.get" == mcp_tool_call_event->name) {
        const auto state = digitalRead(kLedPin) == HIGH;
        printf("on mcp tool call: self.led.get, state: %d\n", state);
        engine.SendMcpCallResponse(mcp_tool_call_event->id, state);
      }
    }
  }
}