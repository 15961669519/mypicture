//==================================================
//  模块连接：  Nano         蓝牙模块
//               10           TX
//               11           RX
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
SoftwareSerial BT(10, 11); // 定义软串口：D10-RX D11-TX

void setup() {
  Serial.begin(9600);   // 电脑和UNO主控板的通讯波特率
  Serial.println("BT is ready!");
  BT.begin(38400);  // UNO 和蓝牙模块的通讯波特率为38400
  delay(200);
  BT.println("AT+NAME=HKZJ\r\n");    // 设置用户名
  delay(20);
  BT.println("AT+PSWD=\"9999\"\r\n");   // 设置用户密码
  delay(20);
  BT.println("AT+ROLE=0\r\n");    // 设置主从模式为从模式
  delay(20);
  BT.println("AT+CMODE=1\r\n");   // 设置连接模式为任意地址模式
  delay(20);
  BT.println("AT+NAME?\r\n");    // 设置主从模式为从模式
  delay(20);
  BT.println("AT+PSWD\r\n");   // 设置连接模式为任意地址模式
}

void loop() {
  if (BT.available() > 0) {
    Serial.print(char(BT.read()));
  }
}
