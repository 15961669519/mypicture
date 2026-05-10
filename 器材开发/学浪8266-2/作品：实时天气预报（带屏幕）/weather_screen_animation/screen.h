//初始化屏幕
void initScreen() {
  u8g2.begin();
  //定义为UTF8字符集（否则不支持中文）
  u8g2.enableUTF8Print();
  //定义字体（不能随便改）
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);
}

//向屏幕输出内容
void echoScreen(int col, int row, const String text) {
  //定位光标
  u8g2.setCursor(col, row);
  //显示内容
  u8g2.print(text);
  //发送到屏幕
  u8g2.sendBuffer();
}

//刷新时间(秒针和分针的更新频率不同)
void echoDatatime(long currentMillis) {

  if (last_ntp==0 || currentMillis - last_ntp >= interval_ntp) {  
    u8g2.setFont(u8g2_font_profont12_tn);
    last_ntp = currentMillis;
    //显示时间
    loopNtp();
    //定位光标
    u8g2.setCursor(0, 16);
    //显示内容
    u8g2.print(wd.localDate);
    //定位光标
    u8g2.setCursor(75, 16);
    //显示内容
    u8g2.print(wd.localTime);
    u8g2.updateDisplayArea(0, 0, 16, 2);
    u8g2.setFont(u8g2_font_wqy14_t_gb2312);
  }
}


//向屏幕显示天气（该函数在每次天气预报更新后，调用）
void echoWeather() {
  u8g2.clearBuffer();
  //显示天气
  u8g2.setCursor(0, 40);
  u8g2.print(wd.now_text);

  u8g2.setCursor(48, 40);
  u8g2.print(String(wd.now_temp) + "°C");

  /**
   * 温湿两个字，跟着天气一起刷，尽量降低温湿度刷新的区域，提高屏幕响应速度
   */
  u8g2.setCursor(0, 60);
  u8g2.print("温 ");
  
  u8g2.setCursor(40, 60);
  u8g2.print("湿 ");  
  u8g2.sendBuffer();
}
