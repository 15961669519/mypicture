#ifndef PS3_CONTRL_H
#define PS3_CONTRL_H

#include <Arduino.h>
#include <Ps3Controller.h>
#include <esp_now.h>
#include <WiFi.h>

// ESP-NOW 数据结构
typedef struct {
    int lx;         // 左摇杆X轴 (-128~127)
    int ly;         // 左摇杆Y轴 (-128~127)
    int rx;         // 右摇杆X轴 (-128~127)
    int ry;         // 右摇杆Y轴 (-128~127)
    uint16_t buttons; // 按钮状态位图
} esp_now_data_t;

// 按钮位图定义
#define BTN_UP       0x0001
#define BTN_DOWN     0x0002
#define BTN_LEFT     0x0004
#define BTN_RIGHT    0x0008
#define BTN_TRIANGLE 0x0010
#define BTN_CIRCLE   0x0020
#define BTN_CROSS    0x0040
#define BTN_SQUARE   0x0080
#define BTN_L1       0x0100
#define BTN_L2       0x0200
#define BTN_R1       0x0400
#define BTN_R2       0x0800
#define BTN_L3       0x1000
#define BTN_R3       0x2000

// 全局变量声明
extern unsigned long NextTransmitTime;
extern int triangle, circle, cross, square;
extern int up, down, left, right;
extern int l1, l2, r1, r2;
extern char circle_pressed, square_pressed, triangle_pressed;
extern char CurDpad, CurDpad2;
extern char led;
extern char led_pressed;
extern char demo;
extern char demo_pressed;
extern char dongzou[3][2];
extern char moshi[3];
extern char fangxiang[4];
extern uint8_t num_dongzou, num_moshi;

// ESP-NOW 相关变量
extern esp_now_data_t esp_now_data;
extern bool esp_now_connected;

// 函数声明
void check_command();
void notify();
void onConnect();
void onDisConnect();
void ps3_init();

// ESP-NOW 相关函数（重命名为 my_esp_now_init 避免冲突）
void my_esp_now_init();
void process_esp_now_data(const uint8_t *data, int len);
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len);

// 蓝牙数据接收处理函数 (在 robot.ino 中定义)
extern void receive_ps3_Data();
extern unsigned char packetData[];
extern unsigned int packetLengthReceived;

// 帧间隔时间定义
#define REC_FRAMEMILLIS 1000

#endif // PS3_CONTRL_H