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
SoftwareSerial BT(10, 11); // 定义软串口：pin10:RX pin11:TX

#define DELAYTIME 100

void setup() {
  Serial.begin(38400);   // 电脑和UNO主控板的通讯波特率
  Serial.println("BT is ready!");
  BT.begin(38400);  // UNO 和蓝牙模块的通讯波特率为38400
  delay(200);
  BT.println("AT+NAME=c497\r\n");    // 设置用户名
  delay(DELAYTIME);
  BT.println("AT+PSWD=\"1234\"\r\n");   // 设置用户密码
  delay(DELAYTIME);
  BT.println("AT+ROLE=0\r\n");    // 设置用户名
  delay(DELAYTIME);
  BT.println("AT+CMODE=1\r\n");    // 设置用户名
  delay(DELAYTIME);
  BT.println("AT+NAME?\r\n");    // 设置用户名
  delay(DELAYTIME);
  BT.println("AT+PSWD\r\n");    // 设置用户名
  delay(DELAYTIME);

}

void loop() {
  // UNO收到蓝牙模块的信息，输出到串口监视器
    if (Serial.available()>0) {
    char val = Serial.read();
    BT.print(val);
  }

  // UNO收到蓝牙模块的信息，输出到串口监视器
  if (BT.available()>0) {
    char val = BT.read();
    Serial.print(val);
  }
}
