#include <espnow.h>
#include <ESP8266WiFi.h>

//目标MAC地址(B)（该主机只收不发，就不需要定义目标mac地址了）
//uint8_t peer1[] = {0xE8, 0xDB, 0x84, 0x9D, 0xB9, 0x42};


typedef struct struct_message {
  float temperature;  //温度
  float humidity;     //湿度
  int clientID; //来消息的客户端ID
} struct_message;

struct_message myData;
struct_message client2;


#include "screen.h"

// Callback when data is sent
//void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//  Serial.print("Last Packet Send Status: ");
//  if (sendStatus == 0){
//    Serial.println("Delivery success");
//    digitalWrite(LED_BUILTIN, LOW);
//  }
//  else{
//    Serial.println("Delivery fail");
//    digitalWrite(LED_BUILTIN, HIGH);
//  }
//}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  
  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.print("Bytes received: ");

  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);

  
  Serial.println(len);
  Serial.print("client:");
  Serial.print(myData.clientID);
  Serial.print(" T: ");
  Serial.println(myData.temperature);
  Serial.print("H: ");
  Serial.println(myData.humidity);
  Serial.println();

  //输出到屏幕
  u8g2.clearBuffer();
  
  u8g2.drawLine(0, 33, 128, 33);
  u8g2.drawLine(35, 0, 35, 64);
  
  if(myData.clientID==1){
    u8g2.setCursor(0, 23);
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.print("主卧");
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    
    u8g2.setCursor(40, 15);
    u8g2.print("温度：");
    u8g2.print(String(myData.temperature));
    u8g2.setCursor(40, 30);
    u8g2.print("湿度：");
    u8g2.print(String(myData.humidity));
    u8g2.updateDisplayArea(0,0,16,4);
  }
  if(myData.clientID==2){
    u8g2.setCursor(0, 54);
    u8g2.setFont(u8g2_font_wqy16_t_gb2312);
    u8g2.print("客厅");
    
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    
    u8g2.setCursor(40, 46);
    u8g2.print("温度：");
    u8g2.print(String(myData.temperature));
    u8g2.setCursor(40, 61);
    u8g2.print("湿度：");
    u8g2.print(String(myData.humidity)); 
    u8g2.updateDisplayArea(0,4,16,4);
  }
//  u8g2.sendBuffer();
  
//  u8g2.setCursor(0, 30);
//  u8g2.print("温度：");
//  u8g2.print(String(myData.temperature));
//
//  u8g2.setCursor(0, 60);
//  u8g2.print("湿度：");
//  u8g2.print(String(myData.humidity));
//  u8g2.sendBuffer();
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  //初始化屏幕
  initScreen();  
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    echoScreen(10,30,"ESP-NOW Error");
    return;
  }
  echoScreen(10,30,"ESP-NOW Init");

  //将该设备设为控制端（可收可发）如果设为ESP_NOW_ROLE_SLAVE 则只能收。
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  //发送成功后的回调函数（如目标设备离线，则返回错误信息）
//  esp_now_register_send_cb(OnDataSent);

  //接收成功后的回调函数，可以在其中获取接收到的数据
  esp_now_register_recv_cb(OnDataRecv);

  //注册通讯的设备，如果有多个设备，就注册多次
//  esp_now_add_peer(peer1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
}
