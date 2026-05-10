//连接wifi
void initWiFi() {

  Serial.print("Connecting to ");
  Serial.println(ssid);

  //定义工作模式，8266 不仅能连接到WI-FI，也可以建立热点让别人连接自己哟。
  WiFi.mode(WIFI_STA);
  
  //开始连接
  WiFi.begin(ssid, password);

  //检查连接是否成功
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }
  tft.println("");
  tft.setTextColor(TFT_GREEN, TFT_BLACK, true);
  tft.println(WiFi.localIP());

  //设置自动重联
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  //连接完毕，打印自身的IP地址
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}
