#include <IRremote.h>   // 导入红外库

const int RECV_PIN = 12;  // 定义红外模块连接引脚D12
const int redLedPin = 2;     // 红色LED灯连接D2引脚
const int greenLedPin = 6;   // 绿色LED灯连接D6引脚
int flag = 4; // 定义状态变量

IRrecv irrecv(RECV_PIN);  // 定义红外类库对象实例
decode_results results;  // 定位红外接收解码对象实例

void setup()
{
  Serial.begin(9600);   // 设置窗口波特率
  irrecv.enableIRIn(); // 启动红外接收
  pinMode(redLedPin, OUTPUT);   // 设置D2引脚为输出
  pinMode(greenLedPin, OUTPUT);  // 设置D6引脚为输出
}

void loop() {
  if (irrecv.decode(&results)) {   // 接收红外数据，并将解码结果保存到results
    Serial.println(results.value, HEX);  // 将解码结果输出到串口监视器
    if (results.value == 0xFF30CF)
      flag++;
    flag = ( flag > 4) ? 1 : flag;
    irrecv.resume(); // 清空缓存，接收下一个红外数据
  }
  switch (flag) {      // 根据按键值进行判断
    case 1 :             // 如果按下1键
      digitalWrite(redLedPin, HIGH);   // 红色LED灯亮
      digitalWrite(greenLedPin, LOW);  // 绿色LED灯熄灭
      break;
    case 2:
      digitalWrite(redLedPin, LOW);    // 红色LED灯熄灭
      digitalWrite(greenLedPin, HIGH); // 绿色LED灯亮
      break;
    case 3:
      digitalWrite(redLedPin, HIGH);    // 红色LED灯亮
      digitalWrite(greenLedPin, HIGH);  // 绿色LED灯亮
      break;
    case 4:
      digitalWrite(redLedPin, LOW);    // 红色LED灯熄灭
      digitalWrite(greenLedPin, LOW);  // 绿色LED灯熄灭
      break;
  }
}
