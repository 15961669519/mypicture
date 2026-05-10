/*
 * ARDUINO UNO ESP32命令接收测试程序 - 增强版
 * 解决命令混淆问题，添加命令序列验证
 */

#include <SoftwareSerial.h>

// 软串口定义：RX=4, TX=3
SoftwareSerial espSerial(4, 3); // RX, TX

// 全局变量
String currentVerb = "";
String currentNoun = "";
String lastCompleteCommand = "";

unsigned long commandCount = 0;
unsigned long sequenceErrors = 0;
unsigned long lastCommandTime = 0;

// 命令序列状态
enum CommandState {
  WAITING_VERB,
  WAITING_NOUN,
  COMMAND_COMPLETE
};
CommandState cmdState = WAITING_VERB;

// LED引脚
const int ledPin = 13;

void setup() {
  // 初始化硬件串口
  Serial.begin(9600);
  delay(2000);
  
  Serial.println("==========================================");
  Serial.println("ARDUINO UNO ESP32命令接收测试程序 - 增强版");
  Serial.println("版本: 3.0");
  Serial.println("==========================================");
  
  // 初始化软串口
  espSerial.begin(9600);
  
  // 初始化LED引脚
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // 发送就绪信号
  espSerial.println("UNO:READY");
  
  Serial.println("系统已就绪，等待命令...");
  Serial.println("期望的命令序列: VERB -> NOUN -> CMD_END");
  Serial.println();
}

void loop() {
  // 处理来自ESP32的数据
  if (espSerial.available() > 0) {
    processIncomingData();
  }
  
  // 处理串口输入
  if (Serial.available() > 0) {
    processSerialInput();
  }
  
  // 定期显示状态
  static unsigned long lastStatusTime = 0;
  if (millis() - lastStatusTime > 5000) {
    lastStatusTime = millis();
    printStatus();
  }
  
  // 更新LED
  updateLED();
  
  delay(10);
}

// 处理来自ESP32的数据
void processIncomingData() {
  String rawData = espSerial.readStringUntil('\n');
  rawData.trim();
  
  if (rawData.length() == 0) {
    return;
  }
  
  Serial.print("<< 原始数据: ");
  Serial.println(rawData);
  
  // 处理命令序列
  if (rawData.startsWith("VERB:")) {
    processVerbCommand(rawData);
  } 
  else if (rawData.startsWith("NOUN:")) {
    processNounCommand(rawData);
  }
  else if (rawData == "CMD_END") {
    processCmdEnd();
  }
  else if (rawData == "CMD_START") {
    processCmdStart();
  }
  else {
    Serial.print("其他消息: ");
    Serial.println(rawData);
  }
}

// 处理命令开始
void processCmdStart() {
  Serial.println("🚀 开始新命令序列");
  cmdState = WAITING_VERB;
  currentVerb = "";
  currentNoun = "";
}

// 处理动词命令
void processVerbCommand(String command) {
  if (cmdState != WAITING_VERB) {
    Serial.println("⚠️  警告: 在不期望的状态收到VERB");
    sequenceErrors++;
  }
  
  String verb = command.substring(5); // 跳过"VERB:"
  verb.trim();
  
  currentVerb = verb;
  cmdState = WAITING_NOUN;
  
  Serial.print("✅ 收到动词: \"");
  Serial.print(currentVerb);
  Serial.println("\"");
}

// 处理名词命令
void processNounCommand(String command) {
  if (cmdState != WAITING_NOUN) {
    Serial.println("⚠️  警告: 在不期望的状态收到NOUN");
    sequenceErrors++;
  }
  
  String noun = command.substring(5); // 跳过"NOUN:"
  noun.trim();
  
  currentNoun = noun;
  cmdState = COMMAND_COMPLETE;
  
  Serial.print("✅ 收到名词: \"");
  Serial.print(currentNoun);
  Serial.println("\"");
  
  // 显示完整命令
  Serial.print("💡 完整命令: ");
  Serial.print(currentVerb);
  Serial.print(" ");
  Serial.println(currentNoun);
}

// 处理命令结束
void processCmdEnd() {
  if (cmdState != COMMAND_COMPLETE) {
    Serial.print("⚠️  警告: 不完整的命令序列 (状态: ");
    Serial.print(cmdState);
    Serial.println(")");
    sequenceErrors++;
  }
  
  if (currentVerb.length() > 0 && currentNoun.length() > 0) {
    commandCount++;
    lastCommandTime = millis();
    lastCompleteCommand = currentVerb + " " + currentNoun;
    
    Serial.println("🎯 命令序列完成");
    Serial.print("📋 命令 #");
    Serial.print(commandCount);
    Serial.print(": ");
    Serial.println(lastCompleteCommand);
    
    // 执行命令（这里可以添加实际的控制代码）
    executeCommand(currentVerb, currentNoun);
  } else {
    Serial.println("❌ 无效命令: 动词或名词为空");
  }
  
  // 重置状态，等待下一个命令
  cmdState = WAITING_VERB;
  currentVerb = "";
  currentNoun = "";
}

// 执行命令（示例）
void executeCommand(String verb, String noun) {
  Serial.print("⚡ 执行命令: ");
  Serial.print(verb);
  Serial.print(" ");
  Serial.println(noun);
  
  // 这里添加实际的控制逻辑
  if (verb == "打开") {
    if (noun == "风扇") {
      Serial.println("🔌 执行: 打开风扇");
      // digitalWrite(FAN_PIN, HIGH);
    } else if (noun == "电灯") {
      Serial.println("💡 执行: 打开电灯");
      // digitalWrite(LIGHT_PIN, HIGH);
    } else if (noun == "取暖器") {
      Serial.println("🔥 执行: 打开取暖器");
      // digitalWrite(HEATER_PIN, HIGH);
    }
  } else if (verb == "关闭") {
    if (noun == "风扇") {
      Serial.println("🔌 执行: 关闭风扇");
      // digitalWrite(FAN_PIN, LOW);
    } else if (noun == "电灯") {
      Serial.println("💡 执行: 关闭电灯");
      // digitalWrite(LIGHT_PIN, LOW);
    } else if (noun == "取暖器") {
      Serial.println("🔥 执行: 关闭取暖器");
      // digitalWrite(HEATER_PIN, LOW);
    }
  }
  
  // 发送执行反馈给ESP32
  String feedback = "CMD_EXECUTED:" + verb + "_" + noun;
  espSerial.println(feedback);
  Serial.print(">> 反馈: ");
  Serial.println(feedback);
}

// 处理串口输入
void processSerialInput() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.length() == 0) return;
  
  Serial.print(">> 用户命令: ");
  Serial.println(input);
  
  if (input == "s" || input == "status") {
    printStatus();
  }
  else if (input == "c" || input == "clear") {
    cmdState = WAITING_VERB;
    currentVerb = "";
    currentNoun = "";
    Serial.println("✅ 命令状态已清除");
  }
  else if (input == "state") {
    Serial.print("当前命令状态: ");
    switch(cmdState) {
      case WAITING_VERB: Serial.println("等待动词"); break;
      case WAITING_NOUN: Serial.println("等待名词"); break;
      case COMMAND_COMPLETE: Serial.println("命令完成"); break;
    }
  }
  else if (input.startsWith("send ")) {
    String message = input.substring(5);
    espSerial.println(message);
    Serial.print(">> 已发送: ");
    Serial.println(message);
  }
}

// 打印状态信息
void printStatus() {
  Serial.println();
  Serial.println("========== 系统状态 ==========");
  Serial.print("总命令数: ");
  Serial.println(commandCount);
  Serial.print("序列错误: ");
  Serial.println(sequenceErrors);
  
  Serial.print("当前命令状态: ");
  switch(cmdState) {
    case WAITING_VERB: Serial.println("等待动词"); break;
    case WAITING_NOUN: Serial.println("等待名词"); break;
    case COMMAND_COMPLETE: Serial.println("命令完成"); break;
  }
  
  if (lastCommandTime > 0) {
    unsigned long secondsSinceLast = (millis() - lastCommandTime) / 1000;
    Serial.print("最后命令: ");
    Serial.print(secondsSinceLast);
    Serial.println(" 秒前");
    Serial.print("最后完整命令: ");
    Serial.println(lastCompleteCommand);
  }
  
  Serial.println("==============================");
}

// 更新LED状态
void updateLED() {
  static unsigned long lastBlinkTime = 0;
  static bool ledState = LOW;
  
  if (cmdState == WAITING_VERB) {
    // 慢闪：等待命令
    if (millis() - lastBlinkTime > 1000) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  } else if (cmdState == WAITING_NOUN) {
    // 快闪：等待名词
    if (millis() - lastBlinkTime > 300) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  } else {
    // 常亮：命令完成
    digitalWrite(ledPin, HIGH);
  }
}