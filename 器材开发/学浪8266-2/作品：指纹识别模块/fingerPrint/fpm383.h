//定义软串口，指纹模块使用软串口，指纹模组默认57600波特率（单片机内置串口用于和串口监视器通讯，还是115200）
#include <SoftwareSerial.h>
SoftwareSerial Serial2(D1, D2);

bool isSerial = true; //串口是否正确返回数据？true=成功

uint8_t ReceiveBuffer[30] = {0};

uint8_t TouchState = 0;// 1=手指放在指纹上

//灯控指令
const uint8_t BlueLight[] = {0xf1, 0x1f, 0xe2, 0x2e, 0xb6, 0x6b, 0xa8, 0x8a, 0x00, 0x0c, 0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0f, 0x01, 0x04, 0x64, 0x00, 0x00, 0x86};
const uint8_t GreenLight[] = {0xf1, 0x1f, 0xe2, 0x2e, 0xb6, 0x6b, 0xa8, 0x8a, 0x00, 0x0c, 0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0f, 0x01, 0x01, 0x64, 0x00, 0x00, 0x89};
const uint8_t RedLight[] = {0xf1, 0x1f, 0xe2, 0x2e, 0xb6, 0x6b, 0xa8, 0x8a, 0x00, 0x0c, 0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0f, 0x01, 0x02, 0x64, 0x00, 0x00, 0x88};
const uint8_t OffLight[] = {0xf1, 0x1f, 0xe2, 0x2e, 0xb6, 0x6b, 0xa8, 0x8a, 0x00, 0x0c, 0x81, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef};


//进入休眠
const uint8_t sl[] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x08, 0x85, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0C, 0x00, 0xF2};

//指纹匹配
const uint8_t matchFingerPrint[18] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x01, 0x21, 0xDE};

//查询匹配结果
const uint8_t resultFingerPrint[18] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x01, 0x22, 0xDD};

//自动注册
uint8_t regFingerPrint[22] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x0B, 0x82, 0x00, 0x00, 0x00, 0x00, 0x01, 0x18, 0x01, 0x06, 0xFF, 0xFF, 0xE6};

//查询手指在位状态
uint8_t checkFinger[18] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x01, 0x35, 0xCA};

//获取指纹模版数量
uint8_t getFingerTotal[18] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0xFB};

//清除指纹（默认为所有）
uint8_t deleteFinger[21] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x0A, 0x83, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31, 0x01, 0x00, 0x01, 0xCC};

//查询清除指纹是否成功
uint8_t checkDeleteFinger[22] = {0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x0B, 0x82, 0x00, 0x00, 0x00, 0x00, 0x01, 0x32, 0x00, 0x00, 0x00, 0x00, 0xCD};

/**
   接收数据到ReceiveBuffer
*/
void ReceiveData(uint32_t Timeout)
{
  uint8_t i = 0;

  isSerial = true;

  //串口尚不可用，等待串口的数据结果
  while (Serial2.available() == 0 && (--Timeout))
  {
    delay(1);
  }

  if (Timeout <= 0) {
    Serial.println("串口没有接受到任何数据，超时退出");
    isSerial = false;
    return;
  }

  while (Serial2.available() > 0)
  {
    delay(2);
    ReceiveBuffer[i++] = Serial2.read();
    if (i > 27) break;
  }
}
/**
   发送指令
*/
void SendData(const uint8_t buffer[], uint32_t len)
{
  Serial2.write(buffer, len);
  Serial2.flush();
}
/**
 * 1024编程实验室（王亮）为《物联网爱好者》课程编写的代码，用于同学们学习之用。
 * 抖音搜索：1024devel 即可关注我们。
 * 
 * 控制LED灯
*/
void LEDControl(const uint8_t buffer[])
{
  SendData(buffer, 23);
  ReceiveData(2000);
  Serial.println("==to LED==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);
  Serial.println();
}

/**
   睡眠
*/
void fsleep()
{
  SendData(sl, 19);
  ReceiveData(2000);
  Serial.println("==to sleep==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);
  Serial.println();

  if (ReceiveBuffer[20] != 0x00) {
    delay(500);
    fsleep();
  }
}

/**********指纹查询***************************/

//指纹校验结果
bool ReviceCheck() {
  
  //查询匹配结果
  SendData(resultFingerPrint, 18);
  ReceiveData(2000);
  Serial.println("");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);

  Serial.print("结果:");
  Serial.print(ReceiveBuffer[22], HEX);

  Serial.print("分数:");
  Serial.print(ReceiveBuffer[23], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[24], HEX);

  Serial.print("ID:");
  Serial.print(ReceiveBuffer[25], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[26], HEX);
  Serial.println(" ");


  switch (ReceiveBuffer[20]) {
    case 0x00:  //没有报错
      if (ReceiveBuffer[22]) {
        return true;
      }
    case 0x04:  //系统繁忙，自动重试
      Serial.println("系统繁忙，自动重试…");
      delay(200);
      return ReviceCheck();
    case 0x0A:  //指纹库是空的
      Serial.println("ReceiveCheck 无任何可用指纹，请先注册");
      break;
    default:
      Serial.println("ReceiveCheck 查询报错");
      break;
  }
  return false;
}

//指纹校验
bool checkFingerFun() {
  Serial.println("==常规匹配方法==");
  SendData(matchFingerPrint, 18);
  ReceiveData(2000);
  Serial.println("");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);

  switch (ReceiveBuffer[20]) {
    case 0x00:  //没有报错
      return ReviceCheck();
    case 0x04:  //系统繁忙，自动重试
      delay(200);
      return checkFingerFun();
    case 0x1A:
      delay(1000);
      Serial.println("指纹尚未激活，请放手指");
      return checkFingerFun();
    default:
      Serial.println("checkFingerFun 查询报错");
      break;
  }
  return false;
}

//查询手指在位状态
bool checkFingerStateFun() {

  SendData(checkFinger, 18);
  //接收注册返回的指令
  ReceiveData(2000);

  //显示输出的结果
  Serial.println("==查询手指在位状态==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);

  Serial.print("状态:");
  Serial.print(ReceiveBuffer[21], HEX);

  Serial.println(" ");

  //true 表示手指在位
  if (ReceiveBuffer[21] == 0 && ReceiveBuffer[20] == 0x00) {
    return false;
  } else {
    return true;
  }
}

//查询系统内指纹总数
void getFingerTotalFun() {
  SendData(getFingerTotal, 18);

  //接收注册返回的指令
  ReceiveData(2000);

  //显示输出的结果
  Serial.println("==检查指纹模版的总数量==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);


  Serial.print("数量:");
  Serial.print(ReceiveBuffer[21], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[22], HEX);

  Serial.println(" ");
}

//指纹注册
void autoReg() {
  Serial.println("进入注册流程(autoReg)");
  
  //必须触发手指在位的中断，才会继续;否则无限重试
  if (!TouchState) {
    delay(500);
    autoReg();
    return;
  }

  delay(200);

  //发送注册指令
  unsigned char sum = 0;
  for (int i = 15; i <= 20; i++) {
    sum += regFingerPrint[i];
  }
  regFingerPrint[21] = (uint8_t)((~sum) + 1);

  Serial.println("发送注册请求");
  SendData(regFingerPrint, 22);

  while (true) {
    ReceiveData(10000);
    Serial.println("==自动注册结果==");
    Serial.print("命令:");
    Serial.print(ReceiveBuffer[15], HEX);
    Serial.print(" ");
    Serial.print(ReceiveBuffer[16], HEX);

    Serial.print("错误:");
    Serial.print(ReceiveBuffer[20], HEX);

    Serial.print("次数:");
    Serial.print(ReceiveBuffer[21], HEX);

    Serial.print("ID:");
    Serial.print(ReceiveBuffer[22], HEX);
    Serial.print(" ");
    Serial.print(ReceiveBuffer[23], HEX);

    Serial.print("进度:");
    Serial.print(ReceiveBuffer[24], HEX);
    Serial.println(" ");
    
    if ( ReceiveBuffer[21] > 6 || ReceiveBuffer[24] >= 100 ) {
      break;
    }
  }
}

//查询删除指纹的结果
bool checkDeleteFingerFun() {
  SendData(deleteFinger, 22);
  ReceiveData(2000);
  Serial.println("==查询删除指纹命令发送结果==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);

  switch (ReceiveBuffer[20]) {
    case 0x00:  //命令执行成功
      return true;
    case 0x04:  //系统繁忙，自动重试
      delay(300);
      return checkDeleteFingerFun();
    default:
      Serial.println("查询出现错误，详见错误代码");
      break;
  }
  return false;
}

//删除指纹（默认为所有）
bool deleteFingerFun() {
  SendData(deleteFinger, 21);
  ReceiveData(2000);
  Serial.println("==删除指纹命令发送结果==");
  Serial.print("命令:");
  Serial.print(ReceiveBuffer[15], HEX);
  Serial.print(" ");
  Serial.print(ReceiveBuffer[16], HEX);

  Serial.print("错误:");
  Serial.print(ReceiveBuffer[20], HEX);

  switch (ReceiveBuffer[20]) {
    case 0x00:  //命令执行成功
      return checkDeleteFingerFun;
    case 0x04:  //系统繁忙，自动重试
      delay(300);
      return deleteFingerFun();
    case 0x1A:
      Serial.println("请用手指触摸指纹，以激活它");
      delay(1000);
      return deleteFingerFun();
    default:
      Serial.println("查询出现错误");
      break;
  }
  return false;
}
