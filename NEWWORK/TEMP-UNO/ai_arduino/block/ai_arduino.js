(() => {

    'use strict';
    goog.provide('Blockly.Blocks.YANGUILE_AI_ARDUINO');
    goog.require('Blockly.Blocks');

    // 初始化燕归乐AI-Arduino通信
    Blockly.Blocks.yanguile_ai_arduino_init = {
        init: function () {
            this.appendDummyInput("")
                .appendField("初始化燕归乐AI-Arduino通信");
            
            this.appendValueInput("RX_PIN")
                .setCheck(Number)
                .appendField("RX引脚");
            
            this.appendValueInput("TX_PIN")
                .setCheck(Number)
                .appendField("TX引脚");
            
            this.appendValueInput("BAUD")
                .setCheck(Number)
                .appendField("波特率");
            
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(200);
            this.setTooltip('初始化与燕归乐AI模块的软串口通信');
            this.setHelpUrl('');
        }
    };

    // 发送命令到AI模块
    Blockly.Blocks.yanguile_ai_arduino_send = {
        init: function () {
            this.appendDummyInput("")
                .appendField("发送命令到AI模块");
            
            this.appendValueInput("COMMAND")
                .setCheck(String)
                .appendField("命令内容");
            
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(210);
            this.setTooltip('发送命令到燕归乐AI模块（如：播报命令）');
            this.setHelpUrl('');
        }
    };

    // 处理AI命令（整合防错机制）
    Blockly.Blocks.yanguile_ai_arduino_process = {
        init: function () {
            this.appendDummyInput("")
                .appendField("处理AI命令");
            
            this.appendDummyInput()
                .appendField("防重复时间(ms)")
                .appendField(new Blockly.FieldNumber(500, 100, 5000, 100), "DEBOUNCE_TIME");
            
            this.appendDummyInput()
                .appendField("超时时间(ms)")
                .appendField(new Blockly.FieldNumber(2000, 500, 10000, 500), "TIMEOUT");
            
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(220);
            this.setTooltip('处理从AI模块接收的命令，包含完整的防错机制');
            this.setHelpUrl('');
        }
    };

    // 检查是否有有效命令（布尔输出，可与"如果-否则"配合）
    Blockly.Blocks.yanguile_ai_arduino_has_command = {
        init: function () {
            this.appendDummyInput("")
                .appendField("有有效命令");
            
            this.setOutput(true, null);
            this.setInputsInline(true);
            this.setColour(230);
            this.setTooltip('检查是否有有效的动词和名词命令可以处理');
            this.setHelpUrl('');
        }
    };

    // 获取动词
    Blockly.Blocks.yanguile_ai_arduino_get_verb = {
        init: function () {
            this.appendDummyInput("")
                .appendField("获取动词");
            
            this.setOutput(true, null);
            this.setInputsInline(true);
            this.setColour(240);
            this.setTooltip('获取处理后的动词，确保不重复且有效');
            this.setHelpUrl('');
        }
    };

    // 获取名词
    Blockly.Blocks.yanguile_ai_arduino_get_noun = {
        init: function () {
            this.appendDummyInput("")
                .appendField("获取名词");
            
            this.setOutput(true, null);
            this.setInputsInline(true);
            this.setColour(250);
            this.setTooltip('获取处理后的名词，确保不重复且有效');
            this.setHelpUrl('');
        }
    };

    // 清空缓冲区
    Blockly.Blocks.yanguile_ai_arduino_clear = {
        init: function () {
            this.appendDummyInput("")
                .appendField("清空缓冲区");
            
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(260);
            this.setTooltip('清空串口接收缓冲区，防止数据堆积');
            this.setHelpUrl('');
        }
    };

    // 重置命令状态
    Blockly.Blocks.yanguile_ai_arduino_reset = {
        init: function () {
            this.appendDummyInput("")
                .appendField("重置命令状态");
            
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(270);
            this.setTooltip('重置命令处理状态，准备接收新命令');
            this.setHelpUrl('');
        }
    };

    // 新增：检查命令是否超时（布尔输出）
    Blockly.Blocks.yanguile_ai_arduino_is_timeout = {
        init: function () {
            this.appendDummyInput("")
                .appendField("命令是否超时");
            
            this.setOutput(true, null);
            this.setInputsInline(true);
            this.setColour(280);
            this.setTooltip('检查当前命令是否已超时');
            this.setHelpUrl('');
        }
    };

})();