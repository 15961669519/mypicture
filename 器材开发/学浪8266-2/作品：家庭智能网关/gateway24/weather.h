//实时天气
struct weather_data {

  // API状态码，具体含义请参考状态码
  int code = -1;

  //当前API的最近更新时间
  String updateTime = "";

  //数据观测时间
  String now_obsTime = "";

  //温度，默认单位：摄氏度
  String now_temp = "0";

  //体感温度，默认单位：摄氏度
  int now_feelsLike = 0;

  //天气状况和图标的代码，图标可通过天气状况和图标下载
  String now_icon = "";

  //天气状况的文字描述，包括阴晴雨雪等天气状态的描述
  String now_text = "";

  //风向360角度
  String now_wind360 = "-1";

  //风向
  String now_windDir = "";

  //风力等级
  String now_windScale = "-1";

  //风速，公里/小时
  int now_windSpeed = -1;

  //相对湿度，百分比数值
  int now_humidity = -1;

  //当前小时累计降水量，默认单位：毫米
  int now_precip = -1;

  //大气压强，默认单位：百帕
  int now_pressure = -1;

  //能见度，默认单位：公里
  int now_vis = -1;
} wd;


struct weather_tmr_data {
  String fxDate = ""; //"预报日期"
  String sunrise = ""; //"日出时间"
  String sunset = ""; //"日落时间"
  String moonrise = ""; //"月升时间"
  String moonset = ""; //"月落时间"
  String moonPhase = ""; //"月相名称"
  String moonPhaseIcon = ""; //"月相图标代码，图标可通过天气状况和图标下载"
  String tempMax = ""; //"预报当天最高温度"
  String tempMin = ""; //"预报当天最低温度"
  String iconDay = ""; //"预报白天天气状况的图标代码，图标可通过天气状况和图标下载"
  String textDay = ""; //"预报白天天气状况文字描述，包括阴晴雨雪等天气状态的描述"
  String iconNight = ""; //"预报夜间天气状况的图标代码，图标可通过天气状况和图标下载"
  String textNight = ""; //"预报晚间天气状况文字描述，包括阴晴雨雪等天气状态的描述"
  String wind360Day = ""; //"预报白天风向360角度"
  String windDirDay = ""; //"预报白天风向"
  String windScaleDay = ""; //"预报白天风力等级"
  String windSpeedDay = ""; //"预报白天风速，公里/小时"
  String wind360Night = ""; //"预报夜间风向360角度"
  String windDirNight = ""; //"预报夜间当天风向"
  String windScaleNight = ""; //"预报夜间风力等级"
  String windSpeedNight = ""; //"预报夜间风速，公里/小时"
  String precip = ""; //"预报当天总降水量，默认单位：毫米"
  String uvIndex = ""; //"紫外线强度指数"
  String humidity = ""; //"相对湿度，百分比数值"
  String pressure = ""; //"大气压强，默认单位：百帕"
  String vis = ""; //"能见度，默认单位：公里"
};

weather_tmr_data wtd[3];

String windIcon[8] = {"↑", "↗", "→", "↘", "↓", "↙", "←", "↖"};

String getWindIcon(String wind360Day) {
  int d = wind360Day.toInt();
  if (d >= 0 && d < 45) {
    return windIcon[0];
  }
  if (d >= 45 && d < 90) {
    return windIcon[1];
  }
  if (d >= 90 && d < 135) {
    return windIcon[1];
  }
  if (d >= 135 && d < 180) {
    return windIcon[1];
  }
  if (d >= 180 && d < 225) {
    return windIcon[1];
  }
  if (d >= 225 && d < 270) {
    return windIcon[1];
  }
  if (d >= 270 && d < 315) {
    return windIcon[1];
  }
  if (d >= 315 && d <= 360 ) {
    return windIcon[1];
  }
  if (d == -999 ) {
    return "旋";
  }
  if (d == -1 ) {
    return "";
  }
}

String icon(String icon) {
  switch (icon.toInt()) {
    case 100: return "\uf101"; case 101: return "\uf102"; case 102: return "\uf103"; case 103: return "\uf104"; case 104: return "\uf105"; case 150: return "\uf106"; case 151: return "\uf107"; case 152: return "\uf108"; case 153: return "\uf109"; case 300: return "\uf10a"; case 301: return "\uf10b"; case 302: return "\uf10c"; case 303: return "\uf10d"; case 304: return "\uf10e"; case 305: return "\uf10f"; case 306: return "\uf110"; case 307: return "\uf111"; case 308: return "\uf112"; case 309: return "\uf113"; case 310: return "\uf114"; case 311: return "\uf115"; case 312: return "\uf116"; case 313: return "\uf117"; case 314: return "\uf118"; case 315: return "\uf119"; case 316: return "\uf11a"; case 317: return "\uf11b"; case 318: return "\uf11c"; case 350: return "\uf11d"; case 351: return "\uf11e"; case 399: return "\uf11f"; case 400: return "\uf120"; case 401: return "\uf121"; case 402: return "\uf122"; case 403: return "\uf123"; case 404: return "\uf124"; case 405: return "\uf125"; case 406: return "\uf126"; case 407: return "\uf127"; case 408: return "\uf128"; case 409: return "\uf129"; case 410: return "\uf12a"; case 456: return "\uf12b"; case 457: return "\uf12c"; case 499: return "\uf12d"; case 500: return "\uf12e"; case 501: return "\uf12f"; case 502: return "\uf130"; case 503: return "\uf131"; case 504: return "\uf132"; case 507: return "\uf133"; case 508: return "\uf134"; case 509: return "\uf135"; case 510: return "\uf136"; case 511: return "\uf137"; case 512: return "\uf138"; case 513: return "\uf139"; case 514: return "\uf13a"; case 515: return "\uf13b"; case 800: return "\uf13c"; case 801: return "\uf13d"; case 802: return "\uf13e"; case 803: return "\uf13f"; case 804: return "\uf140"; case 805: return "\uf141"; case 806: return "\uf142"; case 807: return "\uf143"; case 900: return "\uf144"; case 901: return "\uf145"; case 999: return "\uf146";
  }
}

//获取天气
void getWeather() {
  //检查WI-FI是否已连接
  if (WiFi.status() == WL_CONNECTED) {
    //准备发起请求
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    //请求接口
    if (https.begin(*client, weather_apiuri + "/weather/now?location=" + cityid + "&key=" + auth + "&gzip=n")) {

      //debug
      Serial.print("[HTTPS] GET...\n");

      //获取HTTP的状态码，一般200=成功，出现其他的比如404、500 均为各种报错
      int httpCode = https.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          //从接口获取数据
          String payload = https.getString();
          //          Serial.println(payload);

          //解析json为数组结构
          DynamicJsonDocument doc(1024);
          //          DeserializationError err = deserializeJson(doc, payload);
          DeserializationError err = deserializeJson(doc, (char*)outbuf1, out_size);
          if ( err.code() == DeserializationError::Ok ) {
            //将数据保存到weahter_data 的结构体，方便后续调用
            wd.code = doc["code"];
            wd.updateTime = doc["updateTime"].as<String>().substring(0, 16);
            wd.now_obsTime = doc["now"]["obsTime"].as<String>().substring(0, 16);

            wd.updateTime.replace("T", " ");
            wd.now_obsTime.replace("T", " ");

            wd.now_temp = doc["now"]["temp"].as<String>();
            wd.now_feelsLike = doc["now"]["feelsLike"].as<int>();
            wd.now_icon = doc["now"]["icon"].as<String>();
            wd.now_text = doc["now"]["text"].as<String>();
            wd.now_wind360 = doc["now"]["wind360"].as<String>();
            wd.now_windDir = doc["now"]["windDir"].as<String>();
            wd.now_windScale = doc["now"]["windScale"].as<String>();
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

          } else {
            Serial.println("数据解析出错");
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  }
}

void getTmrWeather() {
  //检查WI-FI是否已连接
  if (WiFi.status() == WL_CONNECTED) {
    //准备发起请求
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    //请求接口
    if (https.begin(*client, weather_apiuri + "/weather/3d?key=" + auth + "&location=" + cityid + "&gzip=n")) {

      //debug
      Serial.print("[HTTPS] GET...\n");

      //获取HTTP的状态码，一般200=成功，出现其他的比如404、500 均为各种报错
      int httpCode = https.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          //从接口获取数据
          String payload = https.getString();
          Serial.println(payload);

          //解析json为数组结构
          DynamicJsonDocument doc2(3072);
          DeserializationError err2 = deserializeJson(doc2, payload);
          Serial.println(err2.code());
          if ( err2.code() == DeserializationError::Ok ) {
            String fxDate = doc2["daily"][0]["fxDate"];
            Serial.println(fxDate);
            for (uint8 i = 0; i < 3; i++) {
              //将数据保存到weahter_data 的结构体，方便后续调用
              wtd[i].fxDate        = doc2["daily"][i]["fxDate"].as<String>();
              wtd[i].sunrise       = doc2["daily"][i]["sunrise"].as<String>();
              wtd[i].sunset        = doc2["daily"][i]["sunset"].as<String>();
              wtd[i].moonrise      = doc2["daily"][i]["moonrise"].as<String>();
              wtd[i].moonset       = doc2["daily"][i]["moonset"].as<String>();
              wtd[i].moonPhase     = doc2["daily"][i]["moonPhase"].as<String>();
              wtd[i].moonPhaseIcon = doc2["daily"][i]["moonPhaseIcon"].as<String>();
              wtd[i].tempMax       = doc2["daily"][i]["tempMax"].as<String>();
              wtd[i].tempMin       = doc2["daily"][i]["tempMin"].as<String>();
              wtd[i].iconDay       = doc2["daily"][i]["iconDay"].as<String>();
              wtd[i].textDay       = doc2["daily"][i]["textDay"].as<String>();
              wtd[i].iconNight       = doc2["daily"][i]["iconNight"].as<String>();
              wtd[i].textNight       = doc2["daily"][i]["textNight"].as<String>();
              wtd[i].wind360Day      = doc2["daily"][i]["wind360Day"].as<String>();
              wtd[i].windDirDay      = doc2["daily"][i]["windDirDay"].as<String>();
              wtd[i].windScaleDay      = doc2["daily"][i]["windScaleDay"].as<String>();
              wtd[i].windSpeedDay      = doc2["daily"][i]["windSpeedDay"].as<String>();
              wtd[i].wind360Night      = doc2["daily"][i]["wind360Night"].as<String>();
              wtd[i].windDirNight      = doc2["daily"][i]["windDirNight"].as<String>();
              wtd[i].windScaleNight      = doc2["daily"][i]["windScaleNight"].as<String>();
              wtd[i].windSpeedNight      = doc2["daily"][i]["windSpeedNight"].as<String>();
              wtd[i].precip        = doc2["daily"][i]["precip"].as<String>();
              wtd[i].uvIndex       = doc2["daily"][i]["uvIndex"].as<String>();
              wtd[i].humidity      = doc2["daily"][i]["humidity"].as<String>();
              wtd[i].pressure      = doc2["daily"][i]["pressure"].as<String>();
              wtd[i].vis           = doc2["daily"][i]["vis"].as<String>();
            }


          } else {
            Serial.println("数据解析出错");
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  }
}
