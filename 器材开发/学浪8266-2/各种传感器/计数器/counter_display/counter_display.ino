/**
   1024编程实验室，王亮编写于 2023年2月11日
   官方正版抖音号：1024devel（1024编程实验室）、iot1024（创联智能）
   有任何问题，请在学浪班级群给我留言。

   可以输出通用传感器的数字、模拟信号值，方便我们做测试
*/
#include <Arduino.h>

//包含字库
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

int getNumberWidth(long i, int w) {
  if (i < 10) {
    return 1 * w / 2;
  }
  if (i < 100) {
    return 2 * w / 2;
  }
  if (i < 1000) {
    return 3 * w / 2;
  }
  if (i < 10000) {
    return 4 * w / 2;
  }
  if (i < 100000) {
    return 5 * w / 2;
  }
  if (i < 1000000) {
    return 6 * w / 2;
  }
  if (i < 10000000) {
    return 7 * w / 2;
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  //屏幕模块的准备工作
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);

  //清除屏幕内容
  u8g2.clearBuffer();
  //显示标题
  u8g2.setCursor(22, 20);
  u8g2.print("红外计数器");

  //采集数字量信号
  //  u8g2.setCursor(0, 40);
  //  u8g2.print("计数: ");
  u8g2.sendBuffer();
  //  Serial.println(u8g2.getBufferTileWidth());
  u8g2.setFont(u8g2_font_logisoso24_tn);
}

int i = 0;
uint8_t last_d = 1;
unsigned long previousMillis = 0;
const long interval = 200;
unsigned long currentMillis = 0;

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int x = 0;
    int y = 32;
    int f = 24;

    u8g2.setDrawColor(0);
    u8g2.drawBox(x, y, 128, f);
    //    u8g2.sendBuffer();

    Serial.println(getNumberWidth(i, 16));
    u8g2.setCursor(64 - getNumberWidth(i, 16), y + f);
    u8g2.setDrawColor(1);
    u8g2.print(i);
    u8g2.updateDisplayArea(x, y / 8, 128 / 8, f / 8);
  }

  if (digitalRead(D5) == 0) {
    if (last_d == 1) {
      i++;
      last_d = 0;
    }
  } else {
    last_d = 1;
  }
}
