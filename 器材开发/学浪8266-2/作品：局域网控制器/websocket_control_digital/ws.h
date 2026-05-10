//将数字量的针脚信息发给所有客户端
void sendPinDatas() {
  DynamicJsonDocument doc(1024);
  for (uint8_t i = 0; i < 8; i++) {
    doc["d" + String(i + 1)] = pin_data.d[i];
  }
  doc["a0"] = pin_data.a0;
  String json;
  serializeJson(doc, json);
  ws.textAll(json);
}

//响应指定客户端的心跳信息
void sendHeart(AsyncWebSocketClient *client){
  DynamicJsonDocument doc(32);
  doc["millis"] = millis();
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

    //客户端心跳请求
    if (strcmp((char*)data, "heart") == 0) {
      sendHeart(client);
    }
    
    if (strcmp((char*)data, "d1") == 0) {
      digitalWrite( pin_data.dname[0], !pin_data.d[0] );
    }
    if (strcmp((char*)data, "d2") == 0) {
      digitalWrite( pin_data.dname[1], !pin_data.d[1] );
    }
    if (strcmp((char*)data, "d3") == 0) {
      digitalWrite( pin_data.dname[2], !pin_data.d[2] );
    }
    if (strcmp((char*)data, "d4") == 0) {
      digitalWrite( pin_data.dname[3], !pin_data.d[3] );
    }
    if (strcmp((char*)data, "d5") == 0) {
      digitalWrite( pin_data.dname[4], !pin_data.d[4] );
    }
    if (strcmp((char*)data, "d6") == 0) {
      digitalWrite( pin_data.dname[5], !pin_data.d[5] );
    }
    if (strcmp((char*)data, "d7") == 0) {
      digitalWrite( pin_data.dname[6], !pin_data.d[6] );
    }
    if (strcmp((char*)data, "d8") == 0) {
      digitalWrite( pin_data.dname[7], !pin_data.d[7] );
    }
  }
}

//websocket的各种事件
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      sendPinDatas();
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
