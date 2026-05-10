byte readBuffer[5];  // 定义字符数组，用于保存蓝牙模块发送的数据
unsigned int Command;   // 全部变量，无符号整数型，保存整理后的键值
byte slideValue;  // 保存滑动条的数字1~10

void setup() {
  Serial.begin(9600);  // 设置串口波特率
}

void loop() {
  Command = 0;
  getCommand();   // 读取蓝牙模块发送的指令
  Serial.println(Command, HEX);  // 将指令16进制输出到串口监视器
}
//*******************************
//  功能：读取蓝牙模块发送的指令
//  参数：无
//  返回值：通过全局变量传递命令。
//          Command     返回按键的键值
//          slideValue  返回滑动条的键值
//*******************************
void getCommand() {
  byte flag = 1;   // 标记位
  byte num = 0;    // 记录读取字符的个数

  while (flag) {
    if (Serial.available()>0) {         // 判断是否有数据接受
      byte tempBuffer = Serial.read();   // 读取接受的数据
      readBuffer[num] = tempBuffer;      // 将数据保存至数组
      Serial.println(String() + "-----" + tempBuffer + "--" + num);

      num++;
      if (tempBuffer == 0x0D) {           // 判断是否接受的数据是否是回车
        // 读取的数据占用一个字节，command占用两个字节，通过移位指令，将第一个字节存放在command
        // 的高八位，第二个字节存放在第八位。
        Command = (readBuffer[0] << 8) | (readBuffer[1]);
        flag = 0;   // 标志位为零，退出while循环
        if (num == 5) {     // 判断是否是滑动条五位数
          slideValue = readBuffer[2] - 47; // 返回滑动条对应的数值1~10
          Serial.println(slideValue);     // 打印滑动条数值
        }
      }
    }
  }
}
