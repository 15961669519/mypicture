//将数字量的针脚信息发给所有客户端
void sendPinDatas(AsyncWebSocketClient *client = NULL) {
  DynamicJsonDocument doc(512);
  for (uint8_t i = 0; i < 8; i++) {
    doc["d" + String(i + 1)] = pin_data.d[i];
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

/**
   接收到指定引脚的请求后，控制输出的PWM波形
*/
void execPin(uint8_t index, uint16_t value) {
  analogWrite( pin_data.dname[index], value );
  pin_data.d[index] = value;
  sendPinDatas();
}

//响应指定客户端的心跳信息
void reviceHeart(AsyncWebSocketClient *client) {
  DynamicJsonDocument doc(512);
  doc["millis"] = millis();
  for (uint8_t i = 0; i < 8; i++) {
    doc["d" + String(i + 1)] = pin_data.d[i];
  }
  String json;
  serializeJson(doc, json);
  client->text(json);
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
      
    //客户端心跳请求
    if(type=="pwm"){
      String id = doc["id"];
      uint16_t value = doc["value"];

      Serial.println(value);

      if (id == "d1") {
        execPin(0, value);
      }
      if (id == "d2") {
        execPin(1, value);
      }
      if (id == "d3") {
        execPin(2, value);
      }
      if (id == "d4") {
        execPin(3, value);
      }
      if (id == "d5") {
        execPin(4, value);
      }
      if (id == "d6") {
        execPin(5, value);
      }
      if (id == "d7") {
        execPin(6, value);
      }
      if (id == "d8") {
        execPin(7, value);
      }
    }

    //    if (strcmp((char*)data, "d8") == 0) {
    //      digitalWrite( pin_data.dname[7], !pin_data.d[7] );
    //    }
  }
}

//websocket的各种事件
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      sendPinDatas(client);
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
  return String();
}
