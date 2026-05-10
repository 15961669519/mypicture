// 《小星星》
//    by  王孚嘉
//
#define  C1 262
#define  C2 294
#define  C3 330
#define  C4 350
#define  C5 393
#define  C6 441
#define  C7 495
// 以上是C调7个音的频率
#define WHOLE 1
#define HALF 0.5
//定义全拍和半拍
int tune[] =
{
  C1, C1, C5, C5, C6,  C6, C5,
  C4, C4, C3, C3, C2,   C2, C1,
  C5, C5, C4, C4,  C3, C3, C2,
  C5, C5, C4, C4,  C3, C3, C2,
  C1, C1, C5, C5, C6,  C6, C5,
  C4, C4, C3, C3, C2,   C2, C1,
};
//曲子的音符部分，用一个序列定义为tune，整数

float duration[] =
{
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1,
};
//曲子的节拍部分，用一个序列定义为duration，浮点

int length;//定义一个变量，后面用来表示共有多少个音符
int tonePin = 8; //喇叭用8引脚
int switchPin = 2;

void setup() {
  pinMode(switchPin, INPUT);
  length = sizeof(tune) / sizeof(tune[0]); //这里用了一个sizeof函数，可以查出tone序列里有多少个音符
}

void loop() {
  if (!digitalRead(switchPin)) {
    for (int x = 0; x < length; x++) //循环音符的次数
    {
      tone(tonePin, tune[x]); //此函数依次播放tune序列里的数组，即每个音符
      delay(1000 * duration[x]); //每个音符持续的时间，即节拍duration，1000是调整时间的,越大，曲子速度越慢，越小曲子速度越快，自己掌握吧
      noTone(tonePin);//停止当前音符，进入下一音符
    }
    delay(2000);//等待2秒后，循环重新开始
  }
}
