//重排序方法
int sort_desc(const void *cmp1, const void *cmp2)
{
  // 将void类型的指针变量，转为字符类型
  String a = *(String*)cmp1;
  String b = *(String*)cmp2;

  //  int c = a.substring(0,2).toInt();
  //  int d = b.substring(0,2).toInt();

  a.replace(":", "");
  b.replace(":", "");

  int c = a.toInt();
  int d = b.toInt();

  return c - d; //正序
  return c > d ? -1 : (c < d ? 1 : 0);  //倒序
}


//响应指定客户端的心跳信息
void revice(AsyncWebSocketClient *client = NULL) {
  DynamicJsonDocument doc(1024);
  doc["millis"] = millis();

  for (int i = 0; i < pin_data.dcount; i++) {
    doc[pin_data.dkey[i]] = pin_data.d[i];
  }

  String json;

  //如果指定了客户端，就单聊
  if (client) {
    doc["client_id"] = client->id();
    serializeJson(doc, json);
    client->text(json);
  } else { //否则，给所有客户端发消息
    serializeJson(doc, json);
    ws.textAll(json);
  }
}
/**
   接收到指定引脚的请求后，控制输出的PWM波形
*/
void execPin(uint8_t index, uint16_t value) {
  servo_write(index, value);

  //记录数据到目标数组
  pin_data.d[index] = value;

  //将新的状态发给所有客户端
  revice();
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

    //响应心跳
    if (type == "heart") {
      revice(client);
    }

    //响应引脚控制
    if (type == "pwm") {
      String id = doc["id"];
      uint16_t value = doc["value"];

      Serial.println(value);

      //循环所有引脚
      for (int i = 0; i < pin_data.dcount; i++) {
        //根据客户端的消息，找到目标引脚
        if (id == pin_data.dkey[i]) {
          //更新引脚状态
          execPin(i, value);
          //ID只有一个，一旦找到了就退出循环。
          break;
        }
      }
    }
  }
}

//websocket的各种事件
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:  //有新的客户端连接
      revice(client);
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT: //断开链接
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA: //发来数据
      handleWebSocketMessage(arg, data, len, client);
      break;
    case WS_EVT_PONG:   //客户端发来ping请求时
    case WS_EVT_ERROR:  //出现了某种错误
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var) {
  Serial.print("processor: ");
  Serial.println(var);
  return String();
}
