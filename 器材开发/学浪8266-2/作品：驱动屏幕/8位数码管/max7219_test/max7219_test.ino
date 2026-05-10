#include "LedControl.h"

/**
 * 初始化对象
 *  LedControl(DIN, CS, CLK, 芯片数量)
 */
LedControl lc = LedControl(D8, D7, D6, 1);

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Serial.println("");

  lc.shutdown(0, false);  // 启用显示
  lc.setIntensity(0, 3); // 设置亮度级别（0-15）

}

void loop() {
  
  //清除显示
  lc.clearDisplay(0);
  
  //循环0-7
  for (int i = 0; i < 8; i++) {
    
    //显示内容，false=表示不显示小数点
    lc.setDigit(0, i, i, false);
    
    //稍等一下
    delay(300);
  }
}
