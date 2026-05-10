#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2c_master.h>
#include <esp_lcd_io_i2c.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_ssd1306.h>
#include <WiFiManager.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

#include "ai_vox_engine.h"
#include "ai_vox_observer.h"
#include "display.h"
#include "audio_device/audio_input_device_i2s_std.h"
#include "audio_device/audio_output_device_i2s_std.h"

#ifndef ARDUINO_ESP32_DEV
#error "This example只支持 ESP32-Dev 板"
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

// 与Arduino UNO通信的串口引脚
constexpr gpio_num_t kUnoTxPin = GPIO_NUM_13;  // ESP32 TX -> UNO RX (引脚4)
constexpr gpio_num_t kUnoRxPin = GPIO_NUM_14;  // ESP32 RX -> UNO TX (引脚3)

constexpr uint32_t kDisplayWidth = 128;
constexpr uint32_t kDisplayHeight = 64;
constexpr bool kDisplayMirrorX = true;
constexpr bool kDisplayMirrorY = true;

std::unique_ptr<Display> g_display;
auto g_observer = std::make_shared<ai_vox::Observer>();
auto g_audio_output_device = std::make_shared<ai_vox::I2sStdAudioOutputDevice>(kSpeakerPinBclk, kSpeakerPinWs, kSpeakerPinDout);

// 配网相关变量
WiFiConfigState g_wifi_config_state = WiFiConfigState::IDLE;
unsigned long g_config_start_time = 0;
constexpr unsigned long kConfigCheckInterval = 50;
constexpr unsigned long kConfigButtonPressTime = 5000;
constexpr unsigned long kConfigPortalTimeout = 180000;
WiFiManager *g_wifiManager = nullptr;

// Arduino UNO通信相关变量
HardwareSerial unoSerial(2); // 使用Serial2
bool unoConnected = false;

// 用于接收UNO数据的缓冲区
std::string g_unoReceiveBuffer;
constexpr size_t kUnoBufferSize = 256;

// UNO命令处理
bool g_has_pending_uno_command = false;
std::string g_pending_uno_command;
unsigned long g_uno_command_time = 0;
constexpr unsigned long kUnoCommandTimeout = 10000; // 10秒超时

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

// 初始化与UNO的串口通信
void InitUnoCommunication() {
  Serial.println("初始化UNO串口通信");
  unoSerial.begin(9600, SERIAL_8N1, kUnoRxPin, kUnoTxPin);
  delay(100);
  
  // 发送测试连接
  unoSerial.print("TEST:ESP32_CONNECTED\n");
  delay(100);
  
  unoConnected = true;
  Serial.println("UNO串口已初始化");
  g_display->ShowStatus("UNO已连接");
}

// 辅助函数：判断一个字节是否是UTF-8中文字符的开始
bool isChineseStartByte(unsigned char c) {
  // UTF-8中文字符的起始字节通常是0xE0-0xEF范围
  return (c >= 0xE0 && c <= 0xEF);
}

// 辅助函数：获取中文字符的字节长度
size_t getChineseCharLength(const std::string& str, size_t start_pos) {
  if (start_pos >= str.length()) return 0;
  
  unsigned char first_byte = static_cast<unsigned char>(str[start_pos]);
  
  // 判断是否是中文字符（UTF-8编码）
  if ((first_byte & 0xE0) == 0xE0) {
    // 中文字符通常是3字节
    if (start_pos + 2 < str.length()) {
      // 检查后续字节是否是有效的UTF-8继续字节
      unsigned char second_byte = static_cast<unsigned char>(str[start_pos + 1]);
      unsigned char third_byte = static_cast<unsigned char>(str[start_pos + 2]);
      
      if ((second_byte & 0xC0) == 0x80 && (third_byte & 0xC0) == 0x80) {
        return 3;
      }
    }
  }
  
  // 如果不是中文字符，返回1（单字节字符）
  return 1;
}

// 处理"马上"命令：格式为"马上XXY"，其中XX是双字动词，Y是对象名词
void processMashangCommand(const std::string& content) {
  // 检查是否包含"马上"
  std::string mashang = "马上";
  size_t mashangPos = content.find(mashang);
  if (mashangPos == std::string::npos) {
    return;
  }
  
  Serial.print("检测到'马上'命令: ");
  Serial.println(content.c_str());
  
  // 计算"马上"的字节数（每个中文字符3字节）
  size_t mashangBytes = 6; // "马上" = 2个中文字符 * 3字节
  
  // 获取"马上"后面的内容
  size_t verbStartPos = mashangPos + mashangBytes;
  
  if (verbStartPos >= content.length()) {
    Serial.println("命令内容不足，无法提取动词");
    return;
  }
  
  // 提取动词（双字动词，2个中文字符）
  std::string verb;
  size_t verbBytes = 0;
  
  // 提取第一个中文字符
  size_t char1Len = getChineseCharLength(content, verbStartPos);
  if (char1Len == 3) {
    verb = content.substr(verbStartPos, 3);
    verbBytes += 3;
    
    // 提取第二个中文字符
    size_t char2StartPos = verbStartPos + 3;
    if (char2StartPos < content.length()) {
      size_t char2Len = getChineseCharLength(content, char2StartPos);
      if (char2Len == 3) {
        verb += content.substr(char2StartPos, 3);
        verbBytes += 3;
      } else {
        // 第二个字符不是中文字符，可能有问题
        Serial.println("警告：动词第二个字符不是有效的中文字符");
        return;
      }
    } else {
      Serial.println("命令长度不足，无法提取双字动词");
      return;
    }
  } else {
    Serial.println("动词第一个字符不是有效的中文字符");
    return;
  }
  
  // 提取名词（剩余所有字符）
  size_t nounStartPos = verbStartPos + verbBytes;
  std::string noun;
  
  if (nounStartPos < content.length()) {
    noun = content.substr(nounStartPos);
    
    // 清理名词末尾的标点符号
    const std::string punctuations = "。，！？.?!;";
    while (!noun.empty()) {
      size_t lastCharLen = getChineseCharLength(noun, noun.length() - 1);
      std::string lastChar = noun.substr(noun.length() - lastCharLen);
      
      if (punctuations.find(lastChar) != std::string::npos) {
        // 去除标点
        noun = noun.substr(0, noun.length() - lastCharLen);
      } else {
        break;
      }
    }
    
    // 去除空白字符
    noun.erase(std::remove_if(noun.begin(), noun.end(), ::isspace), noun.end());
  }
  
  Serial.print("解析结果 -> 动词: ");
  Serial.print(verb.c_str());
  Serial.print(", 名词: ");
  Serial.println(noun.c_str());
  
  // 发送到UNO
  if (!unoConnected) {
    Serial.println("UNO未连接，无法发送命令");
    g_display->ShowStatus("UNO未连接");
    return;
  }
  
  // 分别发送动词和名词
  if (!verb.empty()) {
    std::string verbMsg = "VERB:" + verb + "\n";
    unoSerial.print(verbMsg.c_str());
    Serial.print("发送动词: ");
    Serial.println(verb.c_str());
  }
  
  delay(10); // 短暂延迟确保串口缓冲区清空
  
  if (!noun.empty()) {
    std::string nounMsg = "NOUN:" + noun + "\n";
    unoSerial.print(nounMsg.c_str());
    Serial.print("发送名词: ");
    Serial.println(noun.c_str());
  } else {
    // 如果没有名词，发送一个空的NOUN命令
    unoSerial.print("NOUN:\n");
    Serial.println("发送空名词");
  }
  
  // 在显示屏上显示
  std::string displayMsg = "发送: " + verb;
  if (!noun.empty()) {
    displayMsg += "->" + noun;
  }
  g_display->ShowStatus(displayMsg.c_str());
}

// 处理从UNO接收到的数据
void ProcessUnoData() {
  if (!unoConnected) return;
  
  // 读取串口数据
  while (unoSerial.available() > 0) {
    char c = unoSerial.read();
    
    // 检查缓冲区大小，防止溢出
    if (g_unoReceiveBuffer.length() < kUnoBufferSize) {
      g_unoReceiveBuffer += c;
    } else {
      // 缓冲区满，清空并记录错误
      g_unoReceiveBuffer.clear();
      Serial.println("UNO接收缓冲区溢出");
    }
    
    // 检查是否收到完整的行（以换行符结尾）
    if (c == '\n') {
      // 去除换行符和回车符
      std::string line = g_unoReceiveBuffer;
      while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
        line.pop_back();
      }
      
      g_unoReceiveBuffer.clear(); // 清空缓冲区准备接收下一行
      
      if (!line.empty()) {
        Serial.print("从UNO接收到: ");
        Serial.println(line.c_str());
        
        // 检查是否是"播报"命令
        std::string baobaoPrefix = "播报";
        if (line.find(baobaoPrefix) == 0) {
          // 提取播报内容（去掉"播报"前缀）
          std::string content = line.substr(baobaoPrefix.length());
          
          // 清理内容（去除可能的空白字符）
          content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());
          
          if (!content.empty()) {
            Serial.print("UNO请求播报: ");
            Serial.println(content.c_str());
            
            // 保存待处理的命令
            g_has_pending_uno_command = true;
            g_pending_uno_command = content;
            g_uno_command_time = millis();
            
            // 显示状态
            g_display->ShowStatus("收到UNO命令");
            g_display->SetChatMessage("UNO: " + content);
            
            Serial.println("已保存UNO命令，等待处理");
          }
        } else if (line == "UNO:READY") {
          // UNO就绪消息
          Serial.println("UNO已就绪");
          g_display->ShowStatus("UNO已就绪");
        } else {
          // 其他消息，仅记录
          Serial.print("UNO其他消息: ");
          Serial.println(line.c_str());
        }
      }
    }
  }
}

// 直接模拟用户输入到AI引擎
void SimulateUserInput(const std::string& text) {
  Serial.print("模拟用户输入: ");
  Serial.println(text.c_str());
  
  // 显示状态
  g_display->ShowStatus("处理UNO输入...");
  g_display->SetChatMessage("UNO: " + text);
  
  // 创建用户消息事件并推送到观察者
  ai_vox::Observer::ChatMessageEvent userEvent;
  userEvent.role = ai_vox::ChatRole::kUser;
  userEvent.content = text;
  
  // 将事件推送到观察者队列
  g_observer->PushEvent(userEvent);
  
  Serial.println("已发送模拟用户输入事件");
}

// 检查并处理待处理的UNO命令
void CheckUnoCommand() {
  if (!g_has_pending_uno_command) return;
  
  unsigned long now = millis();
  
  // 检查是否超时
  if (now - g_uno_command_time > kUnoCommandTimeout) {
    Serial.println("UNO命令处理超时");
    g_has_pending_uno_command = false;
    g_display->ShowStatus("UNO命令超时");
    return;
  }
  
  // 检查AI是否在聆听中状态
  // 注意：我们不再尝试触发AI，而是直接发送用户输入
  
  // 直接发送用户输入
  SimulateUserInput(g_pending_uno_command);
  
  // 标记为已处理
  g_has_pending_uno_command = false;
}

// 非阻塞配网处理
void ProcessWiFiConfig() {
  switch (g_wifi_config_state) {
    case WiFiConfigState::IDLE:
      if (digitalRead(kTriggerPin) == LOW) {
        g_config_start_time = millis();
        g_wifi_config_state = WiFiConfigState::CHECKING_BUTTON;
        g_display->ShowStatus("请保持按下5秒");
      }
      break;
      
    case WiFiConfigState::CHECKING_BUTTON:
      if (digitalRead(kTriggerPin) == HIGH) {
        g_wifi_config_state = WiFiConfigState::IDLE;
        g_display->ShowStatus("配网取消");
        return;
      }
      
      if (millis() - g_config_start_time > kConfigButtonPressTime) {
        g_wifi_config_state = WiFiConfigState::CONFIG_PORTAL;
        g_display->ShowStatus("进入配网模式");
        
        if (g_wifiManager) {
          delete g_wifiManager;
          g_wifiManager = nullptr;
        }
        g_wifiManager = new WiFiManager();
        
        g_wifiManager->setAPCallback([](WiFiManager *wm) {
          Serial.println("进入配网模式");
        });
        
        g_wifiManager->setConnectTimeout(0);
        g_wifiManager->setConfigPortalTimeout(300);
        g_wifiManager->setConfigPortalBlocking(false);
        
        if (!g_wifiManager->startConfigPortal("AI_VOX_Config")) {
          Serial.println("启动配网门户失败");
        } else {
          Serial.println("配网门户已启动");
        }
        g_config_start_time = millis();
      }
      break;
      
    case WiFiConfigState::CONFIG_PORTAL:
      if (g_wifiManager) {
        g_wifiManager->process();
        
        if (WiFi.status() == WL_CONNECTED) {
          g_wifi_config_state = WiFiConfigState::CONNECTED;
          g_display->ShowStatus("WiFi已连接");
          printf("WiFi已连接. IP: %s\n", WiFi.localIP().toString().c_str());
          
          delete g_wifiManager;
          g_wifiManager = nullptr;
          return;
        }
        
        if (millis() - g_config_start_time > kConfigPortalTimeout) {
          g_display->ShowStatus("配网超时");
          delay(2000);
          
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
  pinMode(kTriggerPin, INPUT_PULLUP);
  
  printf("尝试自动连接已保存的WiFi\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  unsigned long start = millis();
  bool connected = false;
  
  while (millis() - start < 5000) {
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
  
  g_wifi_config_state = WiFiConfigState::IDLE;
  printf("自动连接失败，等待按键配网\n");
  g_display->ShowStatus("请长按按键5秒配网");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  printf("初始化智能命令解析器\n");

  // 第一步：初始化显示
  InitDisplay();

  // 第二步：启动配网流程
  StartWiFiConfig();

  // 第三步：初始化与Arduino UNO的串口通信
  InitUnoCommunication();

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
  
  if (g_wifi_config_state != WiFiConfigState::CONFIG_PORTAL) {
      ai_vox_engine.Start(audio_input_device, g_audio_output_device);
      if (g_wifi_config_state == WiFiConfigState::CONNECTED) {
          g_display->ShowStatus("命令解析器启动中...");
      }
  }
  printf("智能命令解析器已启动\n");
}

void loop() {
    // 处理配网状态机
    if (g_wifi_config_state != WiFiConfigState::CONNECTED) {
        ProcessWiFiConfig();
        delay(kConfigCheckInterval);
        return;
    }
    
    // 配网完成后显示系统就绪
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        g_display->ShowStatus("系统准备就绪");
        g_display->SetChatMessage("等待语音命令");
        Serial.println("系统就绪，等待语音命令...");
    }

    // 强制配网按钮检测
    static unsigned long last_button_check = 0;
    static bool button_pressed = false;
    static unsigned long button_press_start = 0;
    
    if (millis() - last_button_check > 100) {
        last_button_check = millis();
        
        if (digitalRead(kTriggerPin) == LOW) {
            if (!button_pressed) {
                button_pressed = true;
                button_press_start = millis();
            } else {
                if (millis() - button_press_start > kConfigButtonPressTime) {
                    Serial.println("检测到长按，重置WiFi设置");
                    g_display->ShowStatus("重置WiFi...");
                    
                    WiFiManager wifiManager;
                    wifiManager.resetSettings();
                    WiFi.disconnect(true);
                    
                    delay(1000);
                    ESP.restart();
                }
            }
        } else {
            if (button_pressed) {
                button_pressed = false;
            }
        }
    }
    
    // 处理从UNO接收的数据
    ProcessUnoData();
    
    // 检查并处理待处理的UNO命令
    CheckUnoCommand();

    // 处理事件队列
    const auto events = g_observer->PopEvents();
    for (auto& event : events) {
        if (auto activation_event = std::get_if<ai_vox::Observer::ActivationEvent>(&event)) {
            printf("激活码: %s, 消息: %s\n", activation_event->code.c_str(), activation_event->message.c_str());
            std::string statusMsg = "激活设备" + activation_event->code;
            g_display->ShowStatus(statusMsg.c_str());
            g_display->SetChatMessage(activation_event->message.c_str());
        } else if (auto state_changed_event = std::get_if<ai_vox::Observer::StateChangedEvent>(&event)) {
            switch (state_changed_event->new_state) {
                case ai_vox::ChatState::kIdle: {
                    printf("空闲状态\n");
                    g_display->ShowStatus("空闲状态");
                    break;
                }
                case ai_vox::ChatState::kIniting: {
                    printf("初始化中...\n");
                    g_display->ShowStatus("初始化中");
                    break;
                }
                case ai_vox::ChatState::kStandby: {
                    printf("待命状态\n");
                    g_display->ShowStatus("待命状态");
                    break;
                }
                case ai_vox::ChatState::kConnecting: {
                    printf("连接中...\n");
                    g_display->ShowStatus("连接中...");
                    break;
                }
                case ai_vox::ChatState::kListening: {
                    printf("聆听中...\n");
                    g_display->ShowStatus("聆听中");
                    break;
                }
                case ai_vox::ChatState::kSpeaking: {
                    printf("说话中...\n");
                    g_display->ShowStatus("说话中");
                    break;
                }
                default: {
                    break;
                }
            }
        } else if (auto emotion_event = std::get_if<ai_vox::Observer::EmotionEvent>(&event)) {
            printf("情绪: %s\n", emotion_event->emotion.c_str());
            g_display->SetEmotion(emotion_event->emotion.c_str());
        } else if (auto chat_message_event = std::get_if<ai_vox::Observer::ChatMessageEvent>(&event)) {
            switch (chat_message_event->role) {
                case ai_vox::ChatRole::kAssistant: {
                    printf("助手: %s\n", chat_message_event->content.c_str());
                    g_display->SetChatMessage(chat_message_event->content.c_str());
                    
                    // 检查助手回复是否包含"马上"，如果是则处理并发送到UNO
                    processMashangCommand(chat_message_event->content);
                    break;
                }
                case ai_vox::ChatRole::kUser: {
                    printf("用户: %s\n", chat_message_event->content.c_str());
                    // 用户说话只显示，不处理
                    break;
                }
            }
        }
    }
    
    delay(10);
}