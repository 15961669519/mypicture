#include <ESP8266WiFi.h>
#include <espnow.h>

//目标MAC地址(A)
uint8_t peer1[] = {0xDC, 0x4F, 0x22, 0x0A, 0x54, 0x59};

constexpr char WIFI_SSID[] = "1024";

typedef struct struct_message {
  float temperature;  //温度
  float humidity;     //湿度
  String clientID = "客厅"; //自身的ID，不可和其他设备重复
} struct_message;

struct_message myData;

//多久发一次数据
unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

//包含sht30传感器的文件
#include "sht30.h";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
    for (uint8_t i = 0; i < n; i++) {
      if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
        Serial.println(WiFi.channel(i));
        return WiFi.channel(i);
      }
    }
  }
  return 0;
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
//  Serial.println(getWiFiChannel(WIFI_SSID));
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("B Delivery success");
  }
  else {
    Serial.println("B Delivery fail");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //  esp_wifi_set_promiscuous(true);
//    esp_wifi_set_channel(11);
//    wifi_set_channel(11);
  wifi_set_channel(getWiFiChannel(WIFI_SSID));
  //  esp_wifi_set_promiscuous(false);

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
    lastTime = millis();

    myData.temperature = sht_data.temperature;
    myData.humidity = sht_data.humidity;
//    wifi_set_channel(11);
    // 发送数据给其他设备，NULL 表示所有注册的设备，也可以填写设备的peer 地址
    esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));


  }
}
