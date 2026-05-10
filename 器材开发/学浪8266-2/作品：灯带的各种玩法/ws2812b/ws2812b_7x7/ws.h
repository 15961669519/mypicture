//响应指定客户端的心跳信息
void reviceHeart(AsyncWebSocketClient *client = NULL) {
  DynamicJsonDocument doc(900);
  doc["millis"] = millis();
  doc["brightness"] = led_data.brightness;
  for(int i=0; i<WS_NUM_LEDS; i++){
    doc["colors"][i] = led_data.colors[i];
  }
  
  String json;

  //是否指定了客户端
  if(client){
    doc["client_id"] = client->id();
    serializeJson(doc, json);
    client->text(json);
  }else{
    serializeJson(doc, json);
    ws.textAll(json);
  }
}

//接收客户端发来的消息
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len, AsyncWebSocketClient *client) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    
    Serial.println((char*)data);
    
    DynamicJsonDocument doc(512);
    deserializeJson(doc, (char*)data);
    String type = doc["type"];
    
    if(type=="heart"){
      reviceHeart(client);
    }

    //擦除颜色
    if(type=="clear"){
      
      //取出默认颜色
      String colorString = led_data.color.substring(1);
      //循环将每个灯珠都颜色都设置一遍
      for(int i=0;i<WS_NUM_LEDS;i++){
        led_data.colors[i] = led_data.color;
        //计算hex的值
        long rgb = strtol(colorString.c_str(), NULL, 16);
        led_data.colors_hex[i] = rgb;
      }   
      
      //发送心跳给客户端
      reviceHeart();

      //将新的颜色发送给灯珠（内部还有一次for循环，稍微浪费性能，不过代码看起来更清晰，就保留吧）
      ws2812b_color();   
    }
    
    //颜色控制指令
    if(type=="color"){
      String color = doc["color"];
      int id = doc["id"];
      
      led_data.colors[id] = color;
      
      //计算hex的值
      String colorString = led_data.colors[id].substring(1);
      long rgb = strtol(colorString.c_str(), NULL, 16);
      led_data.colors_hex[id] = rgb;
      
      
      Serial.print("id: ");
      Serial.println(id); 
      Serial.print(" color_hex: ");
      Serial.println(led_data.colors_hex[id]);     
      Serial.print(" color: ");
      Serial.println(led_data.colors[id]);

      reviceHeart();
      
      ws2812b_color();
      
    }
      
    //控制亮度
    if(type=="brightness"){
      uint16_t value = doc["value"];
      led_data.brightness = value;
      
      Serial.print("brightness: ");
      Serial.println(led_data.brightness);
      
      ws2812b_brightness(led_data.brightness);
    }
  }
}

//websocket的各种事件
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      reviceHeart(client);
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len, client);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  Serial.println(var);
  if(var == "heart_timeout")
    return String(heart_timeout);
  return String();
}
