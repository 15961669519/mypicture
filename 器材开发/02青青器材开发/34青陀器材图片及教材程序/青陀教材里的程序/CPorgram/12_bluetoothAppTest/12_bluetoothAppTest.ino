//==================================================
//  模块连接：  Nano         蓝牙模块
//               8           TX
//               9           RX
//
//  模拟串口助手，修改蓝牙参数设置
//      1、程序上传
//      2、打开串口监视器，设置串口监视器 参数为 NL+ CR
//      3、指令集： AT
//                 AT+PSWD
//                 AT+NAME=TEAM01   '设置名称为TEAM01
//                 AT+PSWD="2345"   '设置密码为2345
//                 AT+NAME          '查看名称
//                 AT+PSWD          '查看密码
//==================================================

#include <SoftwareSerial.h>   // 引用软串口库
SoftwareSerial BT(10, 11); // 定义软串口：pin10:RX pin11:TX

void setup() {
  Serial.begin(9600);   // 电脑和UNO主控板的通讯波特率
  Serial.println("BT is ready!");
  BT.begin(9600);  // UNO 和蓝牙模块的通讯波特率为38400
  delay(200);
}

void loop() {

  // UNO收到蓝牙模块的信息，输出到串口监视器
  if (BT.available()>0) {
    char val = BT.read();
    Serial.print(val);
  }
}
