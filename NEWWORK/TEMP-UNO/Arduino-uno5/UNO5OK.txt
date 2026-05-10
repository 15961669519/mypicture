#include <SoftwareSerial.h>

SoftwareSerial espSerial(4, 3); // RX=4, TX=3

void setup() {
  espSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println("UNO命令接收器就绪");
  Serial.println("等待ESP32发送命令...");
  Serial.println("==========================");
  
  // 向ESP32发送就绪信号
  espSerial.println("UNO:READY");
  
  Serial.println("说明：本程序可以发送和接收命令");
  Serial.println("命令格式：");
  Serial.println("  发送到ESP32: 播报XXXX");
  Serial.println("  从ESP32接收: VERB:XX 和 NOUN:XX");
  Serial.println("==========================");
  
  // 等待2秒后发送测试命令
  delay(2000);
  Serial.println("发送测试命令到ESP32...");
  espSerial.println("播报今天天气怎么样");
  Serial.println("已发送: 播报今天天气怎么样");
  Serial.println("等待ESP32回复...");
}

void loop() {
  // 发送命令测试（每30秒发送一次）
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 30000) {
    lastSend = millis();
    
    // 发送不同的测试命令
    int cmdNum = random(3);
    switch(cmdNum) {
      case 0:
        espSerial.println("播报打开客厅的灯");
        Serial.println("已发送: 播报打开客厅的灯");
        break;
      case 1:
        espSerial.println("播报今天有什么新闻");
        Serial.println("已发送: 播报今天有什么新闻");
        break;
      case 2:
        espSerial.println("播报讲个笑话");
        Serial.println("已发送: 播报讲个笑话");
        break;
    }
  }
  
  // 检查从ESP32发送的命令
  if (espSerial.available() > 0) {
    String command = espSerial.readStringUntil('\n');
    command.trim();
    
    if (command.length() > 0) {
      // 显示原始命令
      Serial.print("收到原始命令: ");
      Serial.println(command);
      
      // 根据前缀解析命令
      if (command.startsWith("VERB:")) {
        String verb = command.substring(5); // 去掉"VERB:"前缀
        Serial.println("┌─────────────────────┐");
        Serial.println("│     动词接收        │");
        Serial.println("├─────────────────────┤");
        Serial.print("│ 双字动词: ");
        Serial.print(verb);
        // 对齐显示
        int spaces = 13 - verb.length();
        for (int i = 0; i < spaces; i++) Serial.print(" ");
        Serial.println("│");
        Serial.println("└─────────────────────┘");
      }
      else if (command.startsWith("NOUN:")) {
        String noun = command.substring(5); // 去掉"NOUN:"前缀
        if (noun.length() == 0) {
          Serial.println("┌─────────────────────┐");
          Serial.println("│     名词接收        │");
          Serial.println("├─────────────────────┤");
          Serial.println("│ 名词: [空]          │");
          Serial.println("└─────────────────────┘");
        } else {
          Serial.println("┌─────────────────────┐");
          Serial.println("│     名词接收        │");
          Serial.println("├─────────────────────┤");
          Serial.print("│ 对象名词: ");
          Serial.print(noun);
          // 对齐显示
          int spaces = 11 - noun.length();
          for (int i = 0; i < spaces; i++) Serial.print(" ");
          Serial.println("│");
          Serial.println("└─────────────────────┘");
        }
      }
      else if (command.startsWith("TEST:")) {
        Serial.print("测试连接: ");
        Serial.println(command.substring(5));
      }
      else {
        Serial.print("其他消息: ");
        Serial.println(command);
      }
      
      Serial.println("==========================");
      Serial.println("提示：请根据以上双字动词和对象名词添加硬件控制代码");
      Serial.println("==========================");
    }
  }
  
  delay(10);
}