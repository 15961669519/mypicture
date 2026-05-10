//响应指定客户端的心跳信息
void reviceHeart(AsyncWebSocketClient *client = NULL) {
  DynamicJsonDocument doc(512);
  doc["millis"] = millis();
  doc["color"] = led_data.color;
  doc["brightness"] = led_data.brightness;
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
    
    if(type=="color"){
      String color = doc["color"];
      
      led_data.color = color;
      
      Serial.print("color: ");
      Serial.println(led_data.color);
 
      ws2812b_color(led_data.color);
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
