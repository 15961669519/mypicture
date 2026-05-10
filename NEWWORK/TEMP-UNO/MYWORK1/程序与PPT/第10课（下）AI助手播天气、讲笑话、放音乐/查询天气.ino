
#include <SoftwareSerial.h>

// 燕归乐AI-Arduino通信库
SoftwareSerial aiSerial(4, 3);

// 命令存储变量
String yanguile_current_verb = "";
String yanguile_current_noun = "";
String yanguile_last_full_command = "";
unsigned long yanguile_last_command_time = 0;
bool yanguile_has_valid_command = false;

// 命令处理状态
int yanguile_command_state = 0; // 0:等待动词, 1:等待名词, 2:完成

// 全局设置
int yanguile_debounce_ms = 500;
int yanguile_timeout_ms = 2000;

// 防重复验证函数
void processAICommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  // 命令开始标记
  if (cmd == "CMD_START") {
    yanguile_command_state = 0;
    yanguile_current_verb = "";
    yanguile_current_noun = "";
    yanguile_has_valid_command = false;
    return;
  }

  // 处理动词
  if (cmd.startsWith("VERB:")) {
    if (yanguile_command_state != 0) {
      yanguile_command_state = 0; // 状态错误，重置
      return;
    }
    yanguile_current_verb = cmd.substring(5);
    yanguile_current_verb.trim();
    yanguile_command_state = 1;
    return;
  }

  // 处理名词
  if (cmd.startsWith("NOUN:")) {
    if (yanguile_command_state != 1) {
      yanguile_command_state = 0; // 状态错误，重置
      return;
    }
    yanguile_current_noun = cmd.substring(5);
    yanguile_current_noun.trim();
    yanguile_command_state = 2;
    return;
  }

  // 命令结束标记
  if (cmd == "CMD_END") {
    if (yanguile_command_state != 2) {
      yanguile_command_state = 0;
      return;
    }

    // 防重复检查
    String current_cmd = yanguile_current_verb + ":" + yanguile_current_noun;
    unsigned long now_time = millis();

    if (current_cmd == yanguile_last_full_command &&
        (now_time - yanguile_last_command_time) < yanguile_debounce_ms) {
      // 重复命令，忽略
      yanguile_command_state = 0;
      return;
    }

    // 有效命令
    yanguile_last_full_command = current_cmd;
    yanguile_last_command_time = now_time;
    yanguile_has_valid_command = true;
    yanguile_command_state = 0;
    return;
  }
}

// 检查超时函数
bool isCommandTimeout() {
  if (yanguile_last_command_time == 0) return false;
  unsigned long now_time = millis();
  return (now_time - yanguile_last_command_time) > yanguile_timeout_ms;
}

// 检查有效命令函数
bool hasValidCommand() {
  if (isCommandTimeout()) {
    yanguile_has_valid_command = false;
  }
  return yanguile_has_valid_command;
}

// 获取动词函数
String getCurrentVerb() {
  if (hasValidCommand()) {
    return yanguile_current_verb;
  }
  return "";
}

// 获取名词函数
String getCurrentNoun() {
  if (hasValidCommand()) {
    return yanguile_current_noun;
  }
  return "";
}

// 清空缓冲区函数
void clearAIBuffer() {
  while (aiSerial.available() > 0) {
    aiSerial.read();
  }
  yanguile_command_state = 0;
}

// 重置命令函数
void resetAICommand() {
  yanguile_current_verb = "";
  yanguile_current_noun = "";
  yanguile_last_full_command = "";
  yanguile_last_command_time = 0;
  yanguile_has_valid_command = false;
  yanguile_command_state = 0;
}

void setup(){
    aiSerial.begin(9600);
  delay(100);
  // 发送就绪信号
  aiSerial.println("UNO:READY");
  delay(100);

  // 初始化变量
  yanguile_current_verb = "";
  yanguile_current_noun = "";
  yanguile_last_full_command = "";
  yanguile_last_command_time = 0;
  yanguile_has_valid_command = false;
  yanguile_command_state = 0;
  yanguile_debounce_ms = 500;
  yanguile_timeout_ms = 2000;

  {
      String fullCommand = String("播报") + "今天天气怎么样？";
      aiSerial.println(fullCommand);
    }
    delay(10);
  delay(1000);
    {
      String fullCommand = String("播报") + "介绍一下你自己";
      aiSerial.println(fullCommand);
    }
    delay(10);
  delay(1000);
    {
      String fullCommand = String("播报") + "北京哪里好玩呀";
      aiSerial.println(fullCommand);
    }
    delay(10);
}

void loop(){

}