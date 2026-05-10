// 定义 LED的引脚（IO2）
#define LED_BUILTIN 2

void setup() {
  //定义为输出模式
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  //反复切换电平，实现亮灭效果
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);                   
  digitalWrite(LED_BUILTIN, LOW);   
  delay(1000);                    
}
