from machine import Pin
import time

# 定义为输出模式
D4 = Pin(2, Pin.OUT)

# 做个死循环，约等于arduino 的 loop
while(True):

    # 切换为高电平
	D4.on()

    # 等1秒
	time.sleep(1)

    # 切换为低电平
	D4.off()

    # 等1秒
	time.sleep(1)

