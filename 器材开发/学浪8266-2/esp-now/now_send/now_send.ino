#include <espnow.h>
#include <ESP8266WiFi.h>

//接收方的MAC地址(B)
uint8_t peer1[] = {0xE8, 0xDB, 0x84, 0x9D, 0xB9, 0x42};

// 定义一个数据结构，收发双方都要定义相同的结构
typedef struct struct_message {
  int r;
  int g;
  int b;
} struct_message;

//myData 用于保存收到的数据（也用于包装发送的数据）
struct_message myData;

//每隔多久发一次数据
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;  // send readings timer

// 发送数据后会触发该函数
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  // 0= 发送成功，打开LED灯
  if (sendStatus == 0){
    Serial.println("Delivery success");
    digitalWrite(LED_BUILTIN, LOW);
  }
  else{
    // 发送失败，关闭LED灯
    Serial.println("Delivery fail");
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
//收到数据会触发的回调函数
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");

  //收到数据，眨眨眼（连续切换LED灯的亮灭）
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);

  //在串口监视器打印收到的数据
  Serial.println(len);
  Serial.print("r: ");
  Serial.println(myData.r);
  Serial.print("g: ");
  Serial.println(myData.g);
  Serial.print("b: ");
  Serial.println(myData.b);
  Serial.println();
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // 初始化esp-now协议
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
  //定时发送数据
  if ((millis() - lastTime) > timerDelay) {
    // 设定要发送的数据
    myData.r = myData.r==1?0:1; //每次都将r的值，反转。控制接收方LED灯的亮灭
    myData.g = random(0,20);
    myData.b = random(0,20);

    // 发送数据给其他设备，NULL 表示所有注册的设备，也可以填写设备的peer 地址
    esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }
}
