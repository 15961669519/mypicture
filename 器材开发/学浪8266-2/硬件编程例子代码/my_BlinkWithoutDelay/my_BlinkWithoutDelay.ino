/**
 * 不使用delay() 函数实现定时、等待。
 * delay() 函数缺点：会堵塞整个loop，不能同时执行多个任务。
 * 讲解：1024编程实验室 王亮
 */

// 定义常量：LED灯所在针脚，LED_BUILTIN 也是一个常量，由开发板内置。
const int ledPin =  LED_BUILTIN;

// LED针脚的默认电平为：低电平；默认LED是亮的
int ledState = LOW; 

// 定义一个无符号的长整数变量，因为它可能会变的很大
// 将保存上次触发的时间点，默认为0
unsigned long previousMillis = 0;

// 定义常量：触发间隔，单位是毫秒，1000 = 1秒
const long interval = 1000;

void setup() {
  // 设定LED针脚为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {

  /**
   * 获取当前时间。
   * millis() 函数可以获取单片机开机至今的毫秒数；如果开机24小时，就是：60*60*24*1000=86400000 毫秒
   */
  unsigned long currentMillis = millis();

  //如果(当前时间 - 上次触发的时间 > 触发间隔) 说明该进行下一次触发了。
  if (currentMillis - previousMillis >= interval) {
    
    // 将上次触发事件更新为 "当前时间"
    previousMillis = currentMillis;

    /**
     * 这里写任何你想执行的代码
     * 在例子中，则是反转LED的电平，达到亮灭切换的效果
     *    判断LED当前的电平，如果是低，就改为高；如果是高，就改为低
     */
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // 将修改后的电平写入LED针脚
    digitalWrite(ledPin, ledState);
  }
}
