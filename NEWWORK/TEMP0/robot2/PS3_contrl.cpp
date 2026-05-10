#include "PS3_contrl.h"
#include <Arduino.h>
#include <Ps3Controller.h>
#include <esp_now.h>  // 添加ESP-NOW头文件
#include <WiFi.h>     // 添加WiFi头文件

// 全局变量定义
unsigned long NextTransmitTime = 0;
int triangle, circle, cross, square;
int up, down, left, right;
int l1, l2, r1, r2;
char circle_pressed, square_pressed, triangle_pressed;
char CurDpad, CurDpad2;
char led;
char led_pressed;
char demo;
char demo_pressed;
char dongzou[3][2] = {{'@', 'W'},
                      {'@', 'D'},
                      {'@', 'F'}};
char moshi[3] = {'1', '2', '3'};
char fangxiang[4];
uint8_t num_dongzou = 0, num_moshi = 0;

// ESP-NOW 相关变量
esp_now_data_t esp_now_data;
bool esp_now_connected = false;

// ESP-NOW 接收回调函数
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len == sizeof(esp_now_data_t)) {
        memcpy(&esp_now_data, data, len);
        esp_now_connected = true;
        process_esp_now_data(data, len);
    }
}

// 初始化 ESP-NOW
void my_esp_now_init() {
    WiFi.mode(WIFI_STA);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Initialized");
}

// 处理 ESP-NOW 数据
void process_esp_now_data(const uint8_t *data, int len) {
    esp_now_data_t *remote = (esp_now_data_t *)data;
    
    // 更新 PS3 控制数据结构
    Ps3.data.analog.stick.lx = remote->lx;
    Ps3.data.analog.stick.ly = remote->ly;
    Ps3.data.analog.stick.rx = remote->rx;
    Ps3.data.analog.stick.ry = remote->ry;
    
    // 更新按钮状态
    Ps3.data.analog.button.up = (remote->buttons & BTN_UP) ? 255 : 0;
    Ps3.data.analog.button.down = (remote->buttons & BTN_DOWN) ? 255 : 0;
    Ps3.data.analog.button.left = (remote->buttons & BTN_LEFT) ? 255 : 0;
    Ps3.data.analog.button.right = (remote->buttons & BTN_RIGHT) ? 255 : 0;
    Ps3.data.analog.button.triangle = (remote->buttons & BTN_TRIANGLE) ? 255 : 0;
    Ps3.data.analog.button.circle = (remote->buttons & BTN_CIRCLE) ? 255 : 0;
    Ps3.data.analog.button.cross = (remote->buttons & BTN_CROSS) ? 255 : 0;
    Ps3.data.analog.button.square = (remote->buttons & BTN_SQUARE) ? 255 : 0;
    Ps3.data.analog.button.l1 = (remote->buttons & BTN_L1) ? 255 : 0;
    Ps3.data.analog.button.l2 = (remote->buttons & BTN_L2) ? 255 : 0;
    Ps3.data.analog.button.r1 = (remote->buttons & BTN_R1) ? 255 : 0;
    Ps3.data.analog.button.r2 = (remote->buttons & BTN_R2) ? 255 : 0;
    
    // 调用 notify 函数处理数据
    notify();
}

// 检查命令
void check_command() {
    if ((millis() - NextTransmitTime) > REC_FRAMEMILLIS) {
        NextTransmitTime = millis();
        
        if (CurDpad == 'f') {
            packetData[0] = dongzou[num_dongzou][1];
            packetData[1] = moshi[num_moshi];
            packetData[2] = 'f';
        }
        if (CurDpad == 'b') {
            packetData[0] = dongzou[num_dongzou][1];
            packetData[1] = moshi[num_moshi];
            packetData[2] = 'b';
        }
        if (CurDpad == 'l') {
            packetData[0] = dongzou[num_dongzou][1];
            packetData[1] = moshi[num_moshi];
            packetData[2] = 'l';
        }
        if (CurDpad == 'r') {
            packetData[0] = dongzou[num_dongzou][1];
            packetData[1] = moshi[num_moshi];
            packetData[2] = 'r';
        }
        if (CurDpad == 0) {
            packetData[0] = 'w';
            packetData[1] = '1';
            packetData[2] = 's';
        }
        packetLengthReceived = 3;
        receive_ps3_Data();
    }
}

// 连接回调函数
void onConnect() {
    Serial.println("PS3 Controller Connected");
}

// 断开连接回调函数
void onDisConnect() {
    Serial.println("PS3 Controller Disconnected");
    Serial.println("Ready.");
}

// PS3 初始化函数
void ps3_init() {
    Ps3.attach(notify);
    Ps3.attachOnConnect(onConnect);
    Ps3.attachOnDisconnect(onDisConnect);
    Ps3.begin();
    
    // 初始化 ESP-NOW
    my_esp_now_init();
    
    String address = Ps3.getAddress();
    Serial.print("The ESP32's Bluetooth MAC address is: ");
    Serial.println(address);
    Serial.println("Ready.");
    NextTransmitTime = millis();
}

// 处理遥控器输入
void notify() {
    int lx = (Ps3.data.analog.stick.lx);
    int ly = (Ps3.data.analog.stick.ly);
    int rx = (Ps3.data.analog.stick.rx);
    int ry = (Ps3.data.analog.stick.ry);
    up = (Ps3.data.analog.button.up);
    down = (Ps3.data.analog.button.down);
    left = (Ps3.data.analog.button.left);
    right = (Ps3.data.analog.button.right);
    triangle = (Ps3.data.analog.button.triangle);
    circle = (Ps3.data.analog.button.circle);
    cross = (Ps3.data.analog.button.cross);
    square = (Ps3.data.analog.button.square);
    l1 = (Ps3.data.analog.button.l1);
    r1 = (Ps3.data.analog.button.r1);
    l2 = (Ps3.data.analog.button.l2);
    r2 = (Ps3.data.analog.button.r2);
    
    // 打印摇杆数据
    Serial.print("LX: "); Serial.print(lx); 
    Serial.print(" LY: "); Serial.print(ly); 
    Serial.print(" RX: "); Serial.print(rx); 
    Serial.print(" RY: "); Serial.println(ry);
    
    // 打印方向键状态
    Serial.print("D-Pad: ");
    if (CurDpad == 'f') Serial.println("Forward");
    else if (CurDpad == 'b') Serial.println("Backward");
    else if (CurDpad == 'l') Serial.println("Left");
    else if (CurDpad == 'r') Serial.println("Right");
    else Serial.println("Neutral");
    
    Serial.print("Right Stick: ");
    if (CurDpad2 == 'w') Serial.println("Up");
    else if (CurDpad2 == 's') Serial.println("Down");
    else if (CurDpad2 == 'a') Serial.println("Left");
    else if (CurDpad2 == 'd') Serial.println("Right");
    else Serial.println("Neutral");
    
    // 打印按钮状态
    static int lastSquare = 0, lastCircle = 0, lastTriangle = 0;
    static int lastL1 = 0, lastR1 = 0, lastL2 = 0, lastR2 = 0;
    
    if (square != lastSquare) {
        Serial.print("Square: "); 
        Serial.println(square > 240 ? "PRESSED" : "RELEASED");
        lastSquare = square;
    }
    if (circle != lastCircle) {
        Serial.print("Circle: "); 
        Serial.println(circle > 240 ? "PRESSED" : "RELEASED");
        lastCircle = circle;
    }
    if (triangle != lastTriangle) {
        Serial.print("Triangle: "); 
        Serial.println(triangle > 240 ? "PRESSED" : "RELEASED");
        lastTriangle = triangle;
    }
    if (l1 != lastL1) {
        Serial.print("L1: "); Serial.println(l1 > 240 ? "PRESSED" : "RELEASED");
        lastL1 = l1;
    }
    if (r1 != lastR1) {
        Serial.print("R1: "); Serial.println(r1 > 240 ? "PRESSED" : "RELEASED");
        lastR1 = r1;
    }
    if (l2 != lastL2) {
        Serial.print("L2: "); Serial.println(l2 > 240 ? "PRESSED" : "RELEASED");
        lastL2 = l2;
    }
    if (r2 != lastR2) {
        Serial.print("R2: "); Serial.println(r2 > 240 ? "PRESSED" : "RELEASED");
        lastR2 = r2;
    }
    
    // 更新方向键状态
    if (lx < -50 || left > 240) {
        CurDpad = 'l';
    } else if (lx > 50 || right > 240) {
        CurDpad = 'r';
    } else {
        CurDpad = 0;
    }
    
    if (ly < -50 || up > 240) {
        CurDpad = 'f';
    } else if (ly > 50 || down > 240) {
        CurDpad = 'b';
    } else if (CurDpad == 0) {
        CurDpad = 0;
    }
    
    if (rx < -50) {
        CurDpad2 = 'a';
    } else if (rx > 50) {
        CurDpad2 = 'd';
    } else {
        CurDpad2 = 0;
    }
    
    if (ry < -50) {
        CurDpad2 = 'w';
    } else if (ry > 50) {
        CurDpad2 = 's';
    } else if (CurDpad2 == 0) {
        CurDpad2 = 0;
    }
    
    // 找回 LED 和 DEMO 功能
    if(square > 240) {
        square_pressed = 1;
    } else if(square_pressed == 1) {
        square_pressed = 0;
        num_dongzou++;
        if(num_dongzou == 3) num_dongzou = 0;
    }

    if(circle > 240) {
        circle_pressed = 1;
    } else if(circle_pressed == 1) {
        circle_pressed = 0;
        num_moshi++;
        if(num_moshi == 3) num_moshi = 0;
    }

    if(triangle > 240) {
        triangle_pressed = 1;
    } else if(triangle_pressed == 1) {
        triangle_pressed = 0;
        num_moshi = 0;
        num_dongzou = 0;
    }

    if(r1 > 240) {
        if( demo_pressed == 0) {
            demo_pressed = 1;
        }
    } else if(demo_pressed == 1) {
        if(demo == 0) demo = 1;
        else  demo = 0;
        demo_pressed = 0;
    }
    if(demo == 1) Serial.println("demo on");
    if(demo == 0) Serial.println("demo off");

    if(l1 > 240) {
        if( led_pressed == 0) {
            led_pressed = 1;
        }
    } else if(led_pressed == 1) {
        if(led == 0) led = 1;
        else  led = 0;
        led_pressed = 0;
    }
    if(led == 1) Serial.println("led on");
    if(led == 0) Serial.println("led off");
    
    // 打印模式状态
    //Serial.print("Motion Mode: "); Serial.print(dongzou[num_dongzou][1]);
    //Serial.print(" Submode: "); Serial.println(moshi[num_moshi]);
    //Serial.print("Demo Mode: "); Serial.println(demo ? "ON" : "OFF");
    //Serial.print("LED Status: "); Serial.println(led ? "ON" : "OFF");
    //Serial.println("---------------------");
    //delay(100);
}