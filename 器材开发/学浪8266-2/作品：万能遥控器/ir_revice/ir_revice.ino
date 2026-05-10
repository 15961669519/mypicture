#include <Arduino.h>
//#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>



const uint16_t kRecvPin = 14; //D5

const uint32_t kBaudRate = 115200;

const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else   
const uint8_t kTimeout = 15;
#endif

const uint16_t kMinUnknownSize = 12;

//匹配信号协议的误差，默认是25%，一般不需要修改这个值
const uint8_t kTolerancePercentage = kTolerance;

#define LEGACY_TIMING_INFO false

// ==================== 以上是可修改的参数 ====================

// 创建捕获信号的对象
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // 保存采集到的编码结果


void setup() {
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
  
  //如果串口尚未准备好，就等50毫秒后，再次检查
  while (!Serial)
    delay(50);

  //检查健康性
//  assert(irutils::lowLevelSanityCheck() == 0);

//  串口输出一个准备开始的信息
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
  // 忽略打开或关闭脉冲小于最小值的消息
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // 覆盖缺省设置
  irrecv.enableIRIn();  // 开始采集

}

void loop() {
  revice();
}

void revice(){
  
  // 如果采集到了数据
  if (irrecv.decode(&results)) {
    // 显示当前的时间戳（单片机启动至今的毫秒数）
    uint32_t now = millis();
    /*
     * printf() 也是打印输出的函数，支持内容的格式化
     * D_STR_TIMESTAMP 应该是一个库内部定义的常量，输出字符串"Timestamp"
     * %06u 和 %03u 定义数据类型为"无符号的整数" 06 和 03 分别是固定了输出的位数
     * (now/1000 将毫秒转为秒， now%1000 转为秒之后的余数，也就是毫秒数)
     */
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    
    // 如果收到了一个超出缓冲区大小的数据
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    
    // 显示代码的版本
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    
    // 如果修改过公差值，则显示修改后的值
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    
    // 输出基本信息（协议和代码）
    Serial.print(resultToHumanReadableBasic(&results));
    
    // 如果是AC设备，显示我们采集到的所有扩展信息
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) 
      Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // "生成器" 在较长延迟中加一个，可以避免一些BUG

    // 输出原始结果，rawData 的部分
    Serial.println(resultToSourceCode(&results));
    Serial.println();    // 输出一个空行（作为和下一组数据的间隔）
    yield();
  }
}
