/**
   wifi_setup() 函数改为 initWifi() 方便原有程序调用
   增加了很多在屏幕上输出内容的代码
   增加configModeCallback 回调函数，进入配网模式自动执行
*/
#include <WiFiManager.h>

//用于触发配置网络的针脚
#define TRIGGER_PIN A0

// 配置网络的超时时间（芯片会自动重启）
uint ap_timeout = 120;

//设置阻塞模式 true = 非阻塞模式
bool wm_nonblocking = false;

const char* apName = "1024AP";

//用于保存数据的标志
bool shouldSaveConfig = false;

//核心对象
WiFiManager wm;

/**
 * 回调函数，保存时调用
 */
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

/**
   在配网模式下的回调函数，可以在屏幕上显示点好玩的信息
*/
void configModeCallback(WiFiManager *myWiFiManager) {

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK, true);

  tft.println("");
  tft.println("Smart Wifi Config");

  tft.println("");
  tft.print("AP: ");
  tft.println(myWiFiManager->getConfigPortalSSID());

  tft.println("");
  tft.print("URL: http://");
  tft.println(WiFi.softAPIP().toString());

  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

//进入配置模式的按钮函数
void toConfigButton() {

  // 检查引脚是否进入低电平
  if ( analogRead(TRIGGER_PIN) > 1000 ) {
    Serial.println("Button Pressed1");
    //信号持续时间（防止误触发）
    delay(3000);

    //50ms后，依然是低电平，我们认为按钮真的被按下了
    if ( analogRead(TRIGGER_PIN) > 1000 ) {
      tft.println("");
      tft.print("SMART START");

      Serial.println("Button Pressed");

      //擦除原来的配置
      wm.resetSettings();
      //重启设备，会自动进入配网模式
      ESP.restart();
    }
  }
}

//配网模式的setup函数
void initSmartWiFi() {

  //用于重置的引脚
  pinMode(TRIGGER_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setSleep(false);
  //设置自动重联
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  //运行模式（阻塞 or 非阻塞）
  if (wm_nonblocking) wm.setConfigPortalBlocking(false);

  //定义配网菜单
  std::vector<const char *> menu = {"wifi", "info", "sep", "restart", "exit"};
  wm.setMenu(menu);

  // 页面风格（黑暗模式）
  wm.setClass("invert");

  //配网超时
  wm.setConfigPortalTimeout(ap_timeout);
  //配网回调
  wm.setAPCallback(configModeCallback);

  //自动联网，联网失败则自动进入配网
  tft.println("connecting to " + WiFi.SSID());
  bool res;
  res = wm.autoConnect(apName);

  if (!res) {
    tft.println("");
    tft.println("connect WI-FI Failed");
    delay(1000);
    Serial.println("Failed to connect or hit timeout");
  }
  else {
    tft.println("");
    tft.println("connected WI-FI");
    Serial.println("connected...yeey :)");
  }
}

//配网循环函数
void wifi_loop() {
  //监听配网按钮
  toConfigButton();
}
