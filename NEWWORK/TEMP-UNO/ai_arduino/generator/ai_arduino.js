(() => {

    'use strict';
    goog.provide('Blockly.Arduino.YANGUILE_AI_ARDUINO');
    goog.require('Blockly.Arduino');

    // 初始化燕归乐AI-Arduino通信
    Blockly.Arduino.forBlock['yanguile_ai_arduino_init'] = function () {
        var rx_pin = Blockly.Arduino.valueToCode(this, 'RX_PIN', Blockly.Arduino.ORDER_ATOMIC) || '4';
        var tx_pin = Blockly.Arduino.valueToCode(this, 'TX_PIN', Blockly.Arduino.ORDER_ATOMIC) || '3';
        var baud = Blockly.Arduino.valueToCode(this, 'BAUD', Blockly.Arduino.ORDER_ATOMIC) || '9600';
        
        // 添加头文件
        Blockly.Arduino.definitions_['include_SoftwareSerial'] = '#include <SoftwareSerial.h>';
        
        // 添加全局变量声明
        Blockly.Arduino.definitions_['var_yanguile_ai_arduino'] = 
            '// 燕归乐AI-Arduino通信库\n' +
            'SoftwareSerial aiSerial(' + rx_pin + ', ' + tx_pin + ');\n' +
            '\n' +
            '// 命令存储变量\n' +
            'String yanguile_current_verb = "";\n' +
            'String yanguile_current_noun = "";\n' +
            'String yanguile_last_full_command = "";\n' +
            'unsigned long yanguile_last_command_time = 0;\n' +
            'bool yanguile_has_valid_command = false;\n' +
            '\n' +
            '// 命令处理状态\n' +
            'int yanguile_command_state = 0; // 0:等待动词, 1:等待名词, 2:完成\n' +
            '\n' +
            '// 全局设置\n' +
            'int yanguile_debounce_ms = 500;\n' +
            'int yanguile_timeout_ms = 2000;\n' +
            '\n' +
            '// 防重复验证函数\n' +
            'void processAICommand(String cmd) {\n' +
            '  cmd.trim();\n' +
            '  if (cmd.length() == 0) return;\n' +
            '  \n' +
            '  // 命令开始标记\n' +
            '  if (cmd == "CMD_START") {\n' +
            '    yanguile_command_state = 0;\n' +
            '    yanguile_current_verb = "";\n' +
            '    yanguile_current_noun = "";\n' +
            '    yanguile_has_valid_command = false;\n' +
            '    return;\n' +
            '  }\n' +
            '  \n' +
            '  // 处理动词\n' +
            '  if (cmd.startsWith("VERB:")) {\n' +
            '    if (yanguile_command_state != 0) {\n' +
            '      yanguile_command_state = 0; // 状态错误，重置\n' +
            '      return;\n' +
            '    }\n' +
            '    yanguile_current_verb = cmd.substring(5);\n' +
            '    yanguile_current_verb.trim();\n' +
            '    yanguile_command_state = 1;\n' +
            '    return;\n' +
            '  }\n' +
            '  \n' +
            '  // 处理名词\n' +
            '  if (cmd.startsWith("NOUN:")) {\n' +
            '    if (yanguile_command_state != 1) {\n' +
            '      yanguile_command_state = 0; // 状态错误，重置\n' +
            '      return;\n' +
            '    }\n' +
            '    yanguile_current_noun = cmd.substring(5);\n' +
            '    yanguile_current_noun.trim();\n' +
            '    yanguile_command_state = 2;\n' +
            '    return;\n' +
            '  }\n' +
            '  \n' +
            '  // 命令结束标记\n' +
            '  if (cmd == "CMD_END") {\n' +
            '    if (yanguile_command_state != 2) {\n' +
            '      yanguile_command_state = 0;\n' +
            '      return;\n' +
            '    }\n' +
            '    \n' +
            '    // 防重复检查\n' +
            '    String current_cmd = yanguile_current_verb + ":" + yanguile_current_noun;\n' +
            '    unsigned long now_time = millis();\n' +
            '    \n' +
            '    if (current_cmd == yanguile_last_full_command && \n' +
            '        (now_time - yanguile_last_command_time) < yanguile_debounce_ms) {\n' +
            '      // 重复命令，忽略\n' +
            '      yanguile_command_state = 0;\n' +
            '      return;\n' +
            '    }\n' +
            '    \n' +
            '    // 有效命令\n' +
            '    yanguile_last_full_command = current_cmd;\n' +
            '    yanguile_last_command_time = now_time;\n' +
            '    yanguile_has_valid_command = true;\n' +
            '    yanguile_command_state = 0;\n' +
            '    return;\n' +
            '  }\n' +
            '}\n' +
            '\n' +
            '// 检查超时函数\n' +
            'bool isCommandTimeout() {\n' +
            '  if (yanguile_last_command_time == 0) return false;\n' +
            '  unsigned long now_time = millis();\n' +
            '  return (now_time - yanguile_last_command_time) > yanguile_timeout_ms;\n' +
            '}\n' +
            '\n' +
            '// 检查有效命令函数\n' +
            'bool hasValidCommand() {\n' +
            '  if (isCommandTimeout()) {\n' +
            '    yanguile_has_valid_command = false;\n' +
            '  }\n' +
            '  return yanguile_has_valid_command;\n' +
            '}\n' +
            '\n' +
            '// 获取动词函数\n' +
            'String getCurrentVerb() {\n' +
            '  if (hasValidCommand()) {\n' +
            '    return yanguile_current_verb;\n' +
            '  }\n' +
            '  return "";\n' +
            '}\n' +
            '\n' +
            '// 获取名词函数\n' +
            'String getCurrentNoun() {\n' +
            '  if (hasValidCommand()) {\n' +
            '    return yanguile_current_noun;\n' +
            '  }\n' +
            '  return "";\n' +
            '}\n' +
            '\n' +
            '// 清空缓冲区函数\n' +
            'void clearAIBuffer() {\n' +
            '  while (aiSerial.available() > 0) {\n' +
            '    aiSerial.read();\n' +
            '  }\n' +
            '  yanguile_command_state = 0;\n' +
            '}\n' +
            '\n' +
            '// 重置命令函数\n' +
            'void resetAICommand() {\n' +
            '  yanguile_current_verb = "";\n' +
            '  yanguile_current_noun = "";\n' +
            '  yanguile_last_full_command = "";\n' +
            '  yanguile_last_command_time = 0;\n' +
            '  yanguile_has_valid_command = false;\n' +
            '  yanguile_command_state = 0;\n' +
            '}\n';
        
        // setup函数代码
        Blockly.Arduino.setups_['setup_yanguile_ai_arduino'] = 
            '  aiSerial.begin(' + baud + ');\n' +
            '  delay(100);\n' +
            '  // 发送就绪信号\n' +
            '  aiSerial.println("UNO:READY");\n' +
            '  delay(100);\n' +
            '  \n' +
            '  // 初始化变量\n' +
            '  yanguile_current_verb = "";\n' +
            '  yanguile_current_noun = "";\n' +
            '  yanguile_last_full_command = "";\n' +
            '  yanguile_last_command_time = 0;\n' +
            '  yanguile_has_valid_command = false;\n' +
            '  yanguile_command_state = 0;\n' +
            '  yanguile_debounce_ms = 500;\n' +
            '  yanguile_timeout_ms = 2000;\n';
        
        return '';
    };

    // 发送命令到AI模块（自动添加"播报"前缀）
    Blockly.Arduino.forBlock['yanguile_ai_arduino_send'] = function () {
        var command = Blockly.Arduino.valueToCode(this, 'COMMAND', Blockly.Arduino.ORDER_ATOMIC) || '""';
        
        // 修复：使用String类正确连接字符串
        var code = 
            '  {\n' +
            '    String fullCommand = String("播报") + ' + command + ';\n' +
            '    aiSerial.println(fullCommand);\n' +
            '  }\n' +
            '  delay(10);\n';
        
        return code;
    };

    // 直接发送命令（无"播报"前缀）
    Blockly.Arduino.forBlock['yanguile_ai_arduino_send_raw'] = function () {
        var command = Blockly.Arduino.valueToCode(this, 'COMMAND', Blockly.Arduino.ORDER_ATOMIC) || '""';
        
        var code = 
            '  aiSerial.println(' + command + ');\n' +
            '  delay(10);\n';
        
        return code;
    };

    // 处理AI命令（整合所有防错机制）
    Blockly.Arduino.forBlock['yanguile_ai_arduino_process'] = function () {
        var debounce_time = this.getFieldValue('DEBOUNCE_TIME') || 500;
        var timeout_time = this.getFieldValue('TIMEOUT') || 2000;
        
        var code = 
            '  // 设置防错参数\n' +
            '  yanguile_debounce_ms = ' + debounce_time + ';\n' +
            '  yanguile_timeout_ms = ' + timeout_time + ';\n' +
            '  \n' +
            '  // 处理所有可用数据\n' +
            '  while (aiSerial.available() > 0) {\n' +
            '    String cmd = aiSerial.readStringUntil(\'\\n\');\n' +
            '    processAICommand(cmd);\n' +
            '  }\n' +
            '  \n' +
            '  // 超时检查\n' +
            '  if (isCommandTimeout() && yanguile_has_valid_command) {\n' +
            '    yanguile_has_valid_command = false;\n' +
            '  }\n' +
            '  \n' +
            '  delay(5); // 短暂延迟\n';
        
        return code;
    };

    // 检查是否有有效命令（布尔输出）
    Blockly.Arduino.forBlock['yanguile_ai_arduino_has_command'] = function () {
        var code = 'hasValidCommand()';
        return [code, Blockly.Arduino.ORDER_ATOMIC];
    };

    // 检查命令是否超时（布尔输出）
    Blockly.Arduino.forBlock['yanguile_ai_arduino_is_timeout'] = function () {
        var code = 'isCommandTimeout()';
        return [code, Blockly.Arduino.ORDER_ATOMIC];
    };

    // 获取动词
    Blockly.Arduino.forBlock['yanguile_ai_arduino_get_verb'] = function () {
        var code = 'getCurrentVerb()';
        return [code, Blockly.Arduino.ORDER_ATOMIC];
    };

    // 获取名词
    Blockly.Arduino.forBlock['yanguile_ai_arduino_get_noun'] = function () {
        var code = 'getCurrentNoun()';
        return [code, Blockly.Arduino.ORDER_ATOMIC];
    };

    // 清空缓冲区
    Blockly.Arduino.forBlock['yanguile_ai_arduino_clear'] = function () {
        var code = 'clearAIBuffer();';
        return code;
    };

    // 重置命令状态
    Blockly.Arduino.forBlock['yanguile_ai_arduino_reset'] = function () {
        var code = 'resetAICommand();';
        return code;
    };

})();