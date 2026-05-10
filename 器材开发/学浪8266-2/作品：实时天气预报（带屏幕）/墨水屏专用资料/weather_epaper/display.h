#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>


//根据自己的屏幕驱动芯片、接线，初始化屏幕
GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> display(GxEPD2_290_T94(/*CS=D8 d1*/ D6, /*DC=D3*/ D3, /*RST=D4*/ D4, /*BUSY=D2 d2*/ D8)); // GDEH0154D67

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

bool isStart = false;

void resultDisplay() {

  //设置要操作的窗口范围（full=整个屏幕）
  if (!isStart) {
    display.setFullWindow();
    isStart = true;
  } else {
    display.setPartialWindow(0, 0, display.width(), display.height());
  }

  //分页缓冲，可以避免单片机内存不足（刷新速度会略慢）
  display.firstPage();
  do
  {
    //要执行的相关操作
    display.fillScreen(GxEPD_WHITE);

    u8g2Fonts.setFont(qweather_font);
    u8g2Fonts.drawUTF8(0, 50, icon(wd.now_icon));
    u8g2Fonts.drawUTF8(0, 100, icon(wtd.iconDay));

    u8g2Fonts.setFont(u8g2_font_wqy14_t_gb2312);

    String weather_today = wd.now_text + " " + String(wd.now_temp) + "°C";
    u8g2Fonts.drawUTF8(50, 10 + 14, weather_today.c_str());

    String wind_today = wd.now_windDir + String(wd.now_windSpeed) + "/Km 湿 " + String(wd.now_humidity) + "%";
    u8g2Fonts.drawUTF8(50, 10 + 14 + 20, wind_today.c_str());

    String weather_tmr = wtd.textDay + " " + wtd.tempMin + "°C 至 " + wtd.tempMax + "°C";
    u8g2Fonts.drawUTF8(50, 60 + 14, weather_tmr.c_str());

    String wind_tmr = wtd.windDirDay + wtd.windSpeedDay + "/Km 湿 " + String(wtd.humidity) + "%";
    u8g2Fonts.drawUTF8(50, 60 + 14 + 20, wind_tmr.c_str());

    u8g2Fonts.setFont(u8g2_font_logisoso18_tf);
    u8g2Fonts.setCursor(180, 20);
    u8g2Fonts.println(nt.localDate);

    u8g2Fonts.setFont(u8g2_font_logisoso38_tf);
    u8g2Fonts.setCursor(165, 70);
    u8g2Fonts.println(nt.localHm);//"11:12"
    
    u8g2Fonts.setFont(u8g2_font_logisoso16_tf);
    u8g2Fonts.setCursor(275, 70);
    u8g2Fonts.println(nt.s);//"秒"
    
    u8g2Fonts.setFont(u8g2_font_wqy14_t_gb2312);
    u8g2Fonts.setCursor(180, 90);
    String week_lunar = weekOfDate1(nt.year, nt.month, nt.day)+" "+outputLunarDate(nt.year, nt.month, nt.day);
    u8g2Fonts.println(week_lunar);

    u8g2Fonts.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2Fonts.setCursor(1, 120);

    String sht30 = "室温" + String(sht_data.temperature) + "°C, 湿度" + String(sht_data.humidity) + "%, ";
    u8g2Fonts.print(sht30);

    String bmp180_sgp30 = "气压" + String(bmp_pa) + ", VOC " + String(sgp.TVOC) + ", CO2 " + String(sgp.eCO2);
    u8g2Fonts.print(bmp180_sgp30);
  }
  while (display.nextPage());
  display.display(true);
  //进入深度睡眠，只能通过RST唤醒？
//  display.powerOff();
//  display.hibernate();
  //深度睡眠10秒
//  ESP.deepSleep(20*1000*1000);
}

void display_init() {

  display.init(115200);

  //设置屏幕方向
  display.setRotation(3);

  //加载字库
  u8g2Fonts.begin(display);
  u8g2Fonts.setFontMode(1);                   // 透明模式（默认如此，并未深究）
  u8g2Fonts.setFontDirection(0);              // 从左到右
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // 文字颜色
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 背景颜色
  //进入深度睡眠
  //  display.hibernate();
}

//开机的过程函数
void display_on() {
  //设置要操作的窗口范围（full=整个屏幕）
  if (!isStart) {
    display.setFullWindow();
    isStart = true;
  } else {
    display.setPartialWindow(0, 0, display.width(), display.height());
  }
  u8g2Fonts.setFont(u8g2_font_wqy14_t_gb2312);
  display.firstPage();
  do
  {
    u8g2Fonts.drawUTF8(20, 60, "系统启动中…");
  }
  while (display.nextPage());
  display.display(true);
}
