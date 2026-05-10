//IIC 协议管理库
#include <Wire.h>

//血氧传感器的驱动库
#include "MAX30100_PulseOximeter.h"

//包含心跳图标
#include "heart.c"
#include "heart_fill.c"

//U8G2 驱动屏幕并显示中文
#include <U8g2lib.h>

//使用默认IIC协议引脚（D1 D2），因此无需定义引脚
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//心跳和血氧的数据保存变量
uint8_t heart = 0;
uint8_t SpO2 = 0;

//刷新频率 1000毫秒
#define REPORTING_PERIOD_MS     1000

// 创建传感器对象
PulseOximeter pox;

// 记录最后一次刷新的时间
uint32_t tsLastReport = 0;

//在屏幕上显示数据
void showScreen(uint8_t pos) {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 30);
  u8g2.print("心率");

  u8g2.setCursor(0, 60);
  u8g2.print("血氧");

  //产生心跳数据时，刷新屏幕上的数据和图标
  if (pos == 0) {
    u8g2.drawXBMP( 64, 32, 32, 32, gImage_heart);
    u8g2.setCursor(40, 30);
    u8g2.print("-");
    u8g2.setCursor(40, 60);
    u8g2.print("-");
  } else {
    u8g2.setCursor(40, 30);
    u8g2.print(heart);
    u8g2.setCursor(40, 60);
    u8g2.print(SpO2);
    u8g2.drawXBMP( 64, 32, 32, 32, gImage_heart_fill);
  }
  u8g2.sendBuffer();
  
  // 下面是局部刷新和分页缓存的代码，暂时不用
  //  u8g2.updateDisplayArea(64/8, 32/8, 32/8, 32/8);
  //  u8g2.firstPage();
  //  do {
  //    u8g2.setCursor(0, 30);
  //    u8g2.print(heart);
  //    u8g2.setCursor(0, 60);
  //    u8g2.print(SpO2);
  //  } while ( u8g2.nextPage() );
}

// 监测到脉冲后的回调函数（你可以理解为心率）
void onBeatDetected() {
  Serial.println("♥ Beat!");
  showScreen(1);
}



void setup() {
  Serial.begin(115200);
  
  //屏幕的初始化工作
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312);

  // 初始化传感器
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  // 设定工作功率
  pox.setIRLedCurrent(MAX30100_LED_CURR_11MA);

  // 注册回调函数
  pox.setOnBeatDetectedCallback(onBeatDetected);


}

void loop() {
  // 读取传感器数据
  pox.update();

  // 按照设定间隔更新数据
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    //保存数据到变量
    heart = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    //输出内容到串口监视器
    Serial.print("Heart rate:");
    Serial.print(heart);
    Serial.print("bpm / SpO2:");
    Serial.print(SpO2);
    Serial.println("%");
    
    showScreen(0);

    tsLastReport = millis();
  }
}
