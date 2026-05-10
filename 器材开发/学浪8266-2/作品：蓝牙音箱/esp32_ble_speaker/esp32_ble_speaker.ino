#include "BluetoothA2DPSink.h"
BluetoothA2DPSink a2dp_sink;

/**
 * 基础版代码比较简单，主要在于元器件的选择和音频质量的调试。
 */
void setup() {
  Serial.begin(115200);
  //配置
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 26, //BCLK
    .ws_io_num = 25,  //LRC
    .data_out_num = 22, //播放声音
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.start("1024Music");
}

void loop() {
  delay(1000);
}
