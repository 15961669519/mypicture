#include <ESP8266WiFi.h>
#include <espnow.h>

//目标MAC地址(A)
uint8_t peer1[] = {0x8C,0xAA,0xB5,0x66,0x7B,0xA2};

typedef struct struct_message {
  float temperature;  //温度
  float humidity;     //湿度
  int clientID = 1; //来消息的客户端ID
} struct_message;

struct_message myData;

//多久发一次数据
unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;

//包含sht30传感器的文件
#include "sht30.h";

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("B Delivery success");
  }
  else{
    Serial.println("B Delivery fail");
  }
}

//接收到数据触发
//void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
//  memcpy(&myData, incomingData, sizeof(myData));
//  Serial.print("Bytes received: ");
//}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
    
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
//  esp_now_register_recv_cb(OnDataRecv);
  
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(peer1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  
  sht30_setup();
}

void loop() {
  
  //读取温湿度数据
  sht30();
  
  if ((millis() - lastTime) > timerDelay) {
    myData.temperature = sht_data.temperature;
    myData.humidity = sht_data.humidity;

    // 发送数据给其他设备，NULL 表示所有注册的设备，也可以填写设备的peer 地址
    esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();

    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
