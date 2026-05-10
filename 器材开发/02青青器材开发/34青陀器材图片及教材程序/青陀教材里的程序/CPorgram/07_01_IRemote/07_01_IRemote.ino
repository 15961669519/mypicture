#include <IRremote.h>   // 导入红外库

const int RECV_PIN = 12;  // 定义红外模块连接引脚D12

IRrecv irrecv(RECV_PIN);  // 定义红外类库对象实例
decode_results results;  // 定位红外接收解码对象实例

void setup()
{
  Serial.begin(9600);   // 设置窗口波特率
  irrecv.enableIRIn(); // 启动红外接收
}

void loop() {
  if (irrecv.decode(&results)) {   // 接收红外数据，并将解码结果保存到results
    Serial.println(results.value, HEX);  // 将解码结果输出到串口监视器
    irrecv.resume(); // 清空缓存，接收下一个红外数据
  }
  delay(100);
}
