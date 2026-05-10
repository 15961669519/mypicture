
//获取天气
void getWeather() {
  
  u8g2.clearBuffer();
  //检查WI-FI是否已连接
  if (WiFi.status() == WL_CONNECTED) {
    //准备发起请求
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    
    Serial.print("[HTTPS] begin...\n");
    echoScreen(0, 20, "请求天气接口...");

    //请求接口
    if (https.begin(*client, "https://devapi.qweather.com/v7/weather/now?key=" + auth + "&location=" + cityid + "&gzip=n")) {
      //debug
      Serial.print("[HTTPS] GET...\n");
      //获取HTTP的状态码，一般200=成功，出现其他的比如404、500 均为各种报错
      int httpCode = https.GET();
      if (httpCode > 0) {
        echoScreen(0, 40, "开始解析数据");
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          //从接口获取数据
          String payload = https.getString();
          Serial.println(payload);

          //解析json为数组结构
          DynamicJsonDocument doc(1024);
          DeserializationError err = deserializeJson(doc, payload);
          if ( err.code() == DeserializationError::Ok ) {
            //将数据保存到weahter_data 的结构体，方便后续调用
            wd.code = doc["code"];
            wd.updateTime = doc["updateTime"].as<String>().substring(0, 16);
            wd.now_obsTime = doc["now"]["obsTime"].as<String>().substring(0, 16);

            wd.updateTime.replace("T", " ");
            wd.now_obsTime.replace("T", " ");

            wd.now_temp = doc["now"]["temp"].as<int>();
            wd.now_feelsLike = doc["now"]["feelsLike"].as<int>();
            wd.now_icon = doc["now"]["icon"].as<String>();
            wd.now_text = doc["now"]["text"].as<String>();
            wd.now_wind360 = doc["now"]["wind360"].as<int>();
            wd.now_windDir = doc["now"]["windDir"].as<String>();
            wd.now_windScale = doc["now"]["windScale"].as<int>();
            wd.now_windSpeed = doc["now"]["windSpeed"].as<int>();
            wd.now_humidity = doc["now"]["humidity"].as<int>();
            wd.now_precip = doc["now"]["precip"].as<int>();
            wd.now_pressure = doc["now"]["pressure"].as<int>();
            wd.now_vis = doc["now"]["vis"].as<int>();

            Serial.print("wd.code: ");
            Serial.println(wd.code);
            
            Serial.print("wd.updateTime: ");
            Serial.println(wd.updateTime);

            Serial.print("wd.now_obsTime: ");
            Serial.println(wd.now_obsTime);

            Serial.print("wd.now_temp: ");
            Serial.println(wd.now_temp);

            Serial.print("wd.now_feelsLike: ");
            Serial.println(wd.now_feelsLike);

            Serial.print("wd.now_icon: ");
            Serial.println(wd.now_icon);

            Serial.print("wd.now_text: ");
            Serial.println(wd.now_text);

            Serial.print("wd.now_wind360: ");
            Serial.println(wd.now_wind360);

            Serial.print("wd.now_windDir: ");
            Serial.println(wd.now_windDir);

            Serial.print("wd.now_windScale: ");
            Serial.println(wd.now_windScale);

            Serial.print("wd.now_windSpeed: ");
            Serial.println(wd.now_windSpeed);

            Serial.print("wd.now_humidity: ");
            Serial.println(wd.now_humidity);

            Serial.print("wd.now_precip: ");
            Serial.println(wd.now_precip);

            Serial.print("wd.now_pressure: ");
            Serial.println(wd.now_pressure);

            Serial.print("wd.now_vis: ");
            Serial.println(wd.now_vis);
            
            echoScreen(0, 60, "数据解析 OK");
            delay(1000);
            
          } else {
            Serial.println("数据解析出错");
            echoScreen(0, 60, "数据解析出错 ERROR");
            delay(1000);
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        echoScreen(0, 40, "请求失败，重启或等几分钟");
        delay(1000);
      }
      https.end();
    }
  } else {
    echoScreen(0, 20, "Wi-Fi 掉线，准备重联");
    delay(1000);
    WiFi.disconnect();
    initWiFi();
  }
}


/**
 * 向屏幕输出天气信息
 */
void echoWeather(){
  //擦除屏幕
  u8g2.clearBuffer();
  
  //定位光标
  u8g2.setCursor(0, 20);
  //显示内容
  u8g2.print(wd.localDate);
  //定位光标
  u8g2.setCursor(75, 20);
  //显示内容
  u8g2.print(wd.localTime);

  u8g2.setCursor(0, 40);
  u8g2.print("气温 " + String(wd.now_temp) + "℃");

  u8g2.setCursor(64, 40);
  u8g2.print("湿度 " + String(wd.now_humidity) + "%");

  u8g2.setCursor(0, 60);
  u8g2.print(wd.now_text);
  u8g2.setCursor(45, 60);
  u8g2.print(wd.now_windDir + "  " + wd.now_windSpeed + " KM");

  //将上面定义的数据，显示到屏幕上
  u8g2.sendBuffer();
}
