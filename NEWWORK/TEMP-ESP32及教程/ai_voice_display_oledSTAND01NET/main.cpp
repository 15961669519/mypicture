#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2c_master.h>
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_ssd1306.h>
#include <WiFiManager.h>

#include "ai_vox_engine.h"
#include "ai_vox_observer.h"
#include "display.h"
#include "i2s_std_audio_input_device.h"
#include "i2s_std_audio_output_device.h"

#ifndef ARDUINO_ESP32_DEV
#error "This example only supports ESP32-Dev board."
#endif

// 配网状态机
enum class WiFiConfigState {
  IDLE,
  CHECKING_BUTTON,
  CONFIG_PORTAL,
  CONNECTED
};

namespace {
constexpr gpio_num_t kMicPinWs = GPIO_NUM_25;
constexpr gpio_num_t kMicPinBclk = GPIO_NUM_26;
constexpr gpio_num_t kMicPinDin = GPIO_NUM_27;

constexpr gpio_num_t kSpeakerPinWs = GPIO_NUM_15;
constexpr gpio_num_t kSpeakerPinBclk = GPIO_NUM_16;
constexpr gpio_num_t kSpeakerPinDout = GPIO_NUM_17;

constexpr gpio_num_t kI2cPinSda = GPIO_NUM_21;
constexpr gpio_num_t kI2cPinScl = GPIO_NUM_22;

constexpr gpio_num_t kTriggerPin = GPIO_NUM_35;
constexpr gpio_num_t kLedPin = GPIO_NUM_2;

constexpr uint32_t kDisplayWidth = 128;
constexpr uint32_t kDisplayHeight = 64;
constexpr bool kDisplayMirrorX = true;
constexpr bool kDisplayMirrorY = true;

std::unique_ptr<Display> g_display;
auto g_observer = std::make_shared<ai_vox::Observer>();
std::shared_ptr<ai_vox::iot::Entity> g_led_iot_entity;
std::shared_ptr<ai_vox::iot::Entity> g_speaker_iot_entity;
auto g_audio_output_device = std::make_shared<ai_vox::I2sStdAudioOutputDevice>(kSpeakerPinBclk, kSpeakerPinWs, kSpeakerPinDout);

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

void InitIot() {
  printf("InitIot\n");
  auto& ai_vox_engine = ai_vox::Engine::GetInstance();

  // Speaker
  std::vector<ai_vox::iot::Property> speaker_properties({
      {"volume", "当前音量值", ai_vox::iot::ValueType::kNumber}
  });

  std::vector<ai_vox::iot::Function> speaker_functions({
      {"SetVolume", "设置音量", {{"volume", "0到100之间的整数", ai_vox::iot::ValueType::kNumber, true}}}
  });

  g_speaker_iot_entity = std::make_shared<ai_vox::iot::Entity>("Speaker", "扬声器", 
                                                               std::move(speaker_properties), 
                                                               std::move(speaker_functions));
  g_speaker_iot_entity->UpdateState("volume", g_audio_output_device->volume());
  ai_vox_engine.RegisterIotEntity(g_speaker_iot_entity);

  // LED - 确保在引擎启动前注册
  std::vector<ai_vox::iot::Property> led_properties({
      {"state", "LED灯开关状态", ai_vox::iot::ValueType::kBool}
  });

  std::vector<ai_vox::iot::Function> led_functions({
      {"TurnOn", "打开LED灯", {}},
      {"TurnOff", "关闭LED灯", {}}
  });

  g_led_iot_entity = std::make_shared<ai_vox::iot::Entity>("Led", "LED灯", 
                                                           std::move(led_properties), 
                                                           std::move(led_functions));
  g_led_iot_entity->UpdateState("state", false);
  ai_vox_engine.RegisterIotEntity(g_led_iot_entity);
}

// 非阻塞配网处理
void ProcessWiFiConfig() {
  switch (g_wifi_config_state) {
    case WiFiConfigState::IDLE:
      // 检查按钮是否按下
      if (digitalRead(kTriggerPin) == LOW) { // 按钮按下
        g_config_start_time = millis();
        g_wifi_config_state = WiFiConfigState::CHECKING_BUTTON;
        g_display->ShowStatus("请保持按下5秒");
        printf("按键按下，检查保持时间\n");
      }
      break;
      
    case WiFiConfigState::CHECKING_BUTTON:
      // 检查按钮是否释放
      if (digitalRead(kTriggerPin) == HIGH) { // 按钮释放
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
  // 配置引脚为上拉输入模式
  pinMode(kTriggerPin, INPUT_PULLUP);
  
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

}  // namespace

void setup() {
  Serial.begin(115200);
  printf("初始化\n");

  // 第一步：初始化显示
  InitDisplay();

  // 第二步：启动配网流程
  StartWiFiConfig();

  // 第三步：初始化IoT实体（确保在引擎启动前注册）
  InitIot();

  // 初始化LED引脚
  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, LOW);

  // 其他硬件初始化
  auto audio_input_device = std::make_shared<ai_vox::I2sStdAudioInputDevice>(kMicPinBclk, kMicPinWs, kMicPinDin);
  auto& ai_vox_engine = ai_vox::Engine::GetInstance();
  ai_vox_engine.SetObserver(g_observer);
  ai_vox_engine.SetTrigger(kTriggerPin);
  ai_vox_engine.SetOtaUrl("https://api.tenclass.net/xiaozhi/ota/");
  ai_vox_engine.ConfigWebsocket("wss://api.tenclass.net/xiaozhi/v1/",
                                {
                                    {"Authorization", "Bearer test-token"},
                                });
  
  // 根据配网状态显示不同信息
  if (g_wifi_config_state != WiFiConfigState::CONFIG_PORTAL) {
      ai_vox_engine.Start(audio_input_device, g_audio_output_device);
      if (g_wifi_config_state == WiFiConfigState::CONNECTED) {
          g_display->ShowStatus("AI Vox 启动中...");
      }
  }
  printf("AI Vox 引擎已启动\n");
}

void loop() {
    // 处理配网状态机
    if (g_wifi_config_state != WiFiConfigState::CONNECTED) {
        ProcessWiFiConfig();
        delay(kConfigCheckInterval);
        return; // 配网过程中跳过其他处理
    }
    
    // 配网完成后显示系统就绪
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        g_display->ShowStatus("系统准备就绪");
    }

    // 添加强制配网按钮检测
    static unsigned long last_button_check = 0;
    static bool button_pressed = false;
    static unsigned long button_press_start = 0;
    
    if (millis() - last_button_check > 100) {
        last_button_check = millis();
        
        if (digitalRead(kTriggerPin) == LOW) { // 按钮按下（低电平）
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

    // 处理事件队列
    const auto events = g_observer->PopEvents();
    for (auto& event : events) {
        if (auto activation_event = std::get_if<ai_vox::Observer::ActivationEvent>(&event)) {
            printf("activation code: %s, message: %s\n", activation_event->code.c_str(), activation_event->message.c_str());
            g_display->ShowStatus((std::string("激活设备") + activation_event->code).c_str());
            g_display->SetChatMessage(activation_event->message);
        } else if (auto state_changed_event = std::get_if<ai_vox::Observer::StateChangedEvent>(&event)) {
            switch (state_changed_event->new_state) {
                case ai_vox::ChatState::kIdle: {
                    printf("Idle\n");
                    break;
                }
                case ai_vox::ChatState::kIniting: {
                    printf("Initing...\n");
                    g_display->ShowStatus("初始化中");
                    break;
                }
                case ai_vox::ChatState::kStandby: {
                    printf("Standby\n");
                    g_display->ShowStatus("待命状态");
                    g_display->SetChatMessage("");
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
        } else if (auto emotion_event = std::get_if<ai_vox::Observer::EmotionEvent>(&event)) {
            printf("emotion: %s\n", emotion_event->emotion.c_str());
            g_display->SetEmotion(emotion_event->emotion);
        } else if (auto chat_message_event = std::get_if<ai_vox::Observer::ChatMessageEvent>(&event)) {
            switch (chat_message_event->role) {
                case ai_vox::ChatRole::kAssistant: {
                    printf("role: assistant, content: %s\n", chat_message_event->content.c_str());
                    break;
                }
                case ai_vox::ChatRole::kUser: {
                    printf("role: user, content: %s\n", chat_message_event->content.c_str());
                    break;
                }
            }
            g_display->SetChatMessage(chat_message_event->content);
        } else if (auto iot_message_event = std::get_if<ai_vox::Observer::IotMessageEvent>(&event)) {
            printf("IOT message: %s, function: %s\n", iot_message_event->name.c_str(), iot_message_event->function.c_str());
            for (const auto& [key, value] : iot_message_event->parameters) {
                if (std::get_if<bool>(&value)) {
                    printf("key: %s, value: %s\n", key.c_str(), std::get<bool>(value) ? "true" : "false");
                } else if (std::get_if<std::string>(&value)) {
                    printf("key: %s, value: %s\n", key.c_str(), std::get<std::string>(value).c_str());
                } else if (std::get_if<int64_t>(&value)) {
                    printf("key: %s, value: %lld\n", key.c_str(), std::get<int64_t>(value));
                }
            }

            if (iot_message_event->name == "Led") {
                if (iot_message_event->function == "TurnOn") {
                    printf("打开LED\n");
                    digitalWrite(kLedPin, HIGH);
                    g_led_iot_entity->UpdateState("state", true);
                } else if (iot_message_event->function == "TurnOff") {
                    printf("关闭LED\n");
                    digitalWrite(kLedPin, LOW);
                    g_led_iot_entity->UpdateState("state", false);
                }
            } else if (iot_message_event->name == "Speaker") {
                if (iot_message_event->function == "SetVolume") {
                    if (const auto it = iot_message_event->parameters.find("volume"); it != iot_message_event->parameters.end()) {
                        auto volume = it->second;
                        if (std::get_if<int64_t>(&volume)) {
                            printf("扬声器音量: %lld\n", std::get<int64_t>(volume));
                            g_audio_output_device->SetVolume(std::get<int64_t>(volume));
                            g_speaker_iot_entity->UpdateState("volume", std::get<int64_t>(volume));
                        }
                    }
                }
            }
        }
    }
    
    // 添加短延时防止任务饥饿
    delay(10);
    taskYIELD();
}