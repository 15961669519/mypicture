#include <ESP8266WiFi.h>
#include <espnow.h>

//接收方的MAC地址(B)
uint8_t peer1[] = {0x8C,0xAA,0xB5,0x66,0x7B,0xA2};

// 定义一个数据结构，收发双方都要定义相同的结构
typedef struct struct_message {
  int r;
  int g;
  int b;
} struct_message;

//myData 用于保存收到的数据（也用于包装发送的数据）
struct_message myData;

// 发送数据后会触发该函数
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("B Delivery success");
  }
  else{
    Serial.println("B Delivery fail");
  }
}

//收到数据会触发的回调函数
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");

  //根据收到的r变量的值，决定是否开灯
  digitalWrite(LED_BUILTIN, myData.r);

  //打印收到的数据
  Serial.println(len);
  Serial.print("r: ");
  Serial.println(myData.r);
  Serial.print("g: ");
  Serial.println(myData.g);
  Serial.print("b: ");
  Serial.println(myData.b);
  Serial.println();

  myData.g = random(0,255);
  
  //收到数据，给对方回一个数据
  Serial.println("sent");
  esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));
}
 
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
  
  //将该设备设为控制端（可收可发）如果设为ESP_NOW_ROLE_SLAVE 则只能收。
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  //发送成功后的回调函数（如目标设备离线，则返回错误信息）
  esp_now_register_send_cb(OnDataSent);

  //接收成功后的回调函数，可以在其中获取接收到的数据
  esp_now_register_recv_cb(OnDataRecv);

  //注册通讯的设备，如果有多个设备，就注册多次
  esp_now_add_peer(peer1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

}

void loop() {
  delay(4000);
}
