// Please tune the following value if the clock gains or loses.
// Theoretically, standard of this value is 60000.
// 如果走时不准，可适当增大或降低这个值
#define MILLIS_PER_MIN 60000 // milliseconds per a minute

// Motor and clock parameters
// 时钟和电机的参数
// 4096 * 90 / 12 = 30720
#define STEPS_PER_ROTATION 30720 // 分针旋转一周的步数

// wait for a single step of stepper
// 步进等待时间
int delaytime = 2;

// ports used to control the stepper motor
// if your motor rotate to the opposite direction, 
// change the order as {2, 3, 4, 5};
// 接电机控制版的4根信号线
int port[4] = {D1, D2, D3, D4};

// sequence of stepper motor control
// 步进电机控制顺序
int seq[8][4] = {
  {  LOW, HIGH, HIGH,  LOW},
  {  LOW,  LOW, HIGH,  LOW},
  {  LOW,  LOW, HIGH, HIGH},
  {  LOW,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW,  LOW},
  { HIGH, HIGH,  LOW,  LOW},
  {  LOW, HIGH,  LOW,  LOW}
};

/**
 * 步进电机的控制函数，约等于一个简易的驱动库
 * 这里的代码细节可以忽略，只要记住怎么用：
 * rotate(4096) 步进电机将旋转1圈
 */
void rotate(int step) {
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : 7;
  int dt = 20;

  step = (step > 0) ? step : -step;
  for(j = 0; j < step; j++) {
    phase = (phase + delta) % 8;
    for(i = 0; i < 4; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    delay(dt);
    if(dt > delaytime) dt--;
  }
  // power cut
  for(i = 0; i < 4; i++) {
    digitalWrite(port[i], LOW);
  }
}

void setup() {
  pinMode(port[0], OUTPUT);
  pinMode(port[1], OUTPUT);
  pinMode(port[2], OUTPUT);
  pinMode(port[3], OUTPUT);
  
  rotate(-20);
  rotate(20);
  
  //开机先转个八分之一圈，意思意思
  rotate(STEPS_PER_ROTATION / 60);
}

void loop() {
  // 
  static long prev_min = 0, prev_pos = 0;
  long min;
  static long pos;
  
  //获取单片机开机至今的分钟数（毫秒/预设的比例值60000）
  min = millis() / MILLIS_PER_MIN;
  
  //单片机每运行1分钟，只触发一次步进电机
  //如果已经触发过了，则直接return，后续代码不再执行
  if(prev_min == min) {
    return;
  }
  
  //将上次触发事件设为当前分钟数
  prev_min = min;

  //计算步进电机累计的步距数
  pos = (STEPS_PER_ROTATION * min) / 60;

  //这两行代码，可能是为了防止步进电机卡住
  rotate(-20);
  rotate(20);

  //用当前的位置值 - 上一次的位置值，就是步进电机该走的距离
  //  1分钟 = 30720*1/60 = 512
  //  2分钟 = 30720*2/60 = 1024
  // 每分钟步进电机运行512的距离（4096=转一圈）
  // 步进电机每旋转八分之一圈，分针在钟盘上转动：十二分之一
  if(pos - prev_pos > 0) {
    rotate(pos - prev_pos);
  }
  prev_pos = pos;
}
