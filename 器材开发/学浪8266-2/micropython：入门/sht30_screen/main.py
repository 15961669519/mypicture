from time import sleep
from sht3x import SHT3x_Sensor
import time

from machine import Pin, I2C
import ssd1306

# oled屏幕对象 默认地址：0x3C
i2c = I2C(sda=Pin(5), scl=Pin(4))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)


# 定义传感器对象(sda\scl为引脚pin)
sht3x_sensor = SHT3x_Sensor(freq=100000, sdapin=12, sclpin=14)

while(True):
    measure_data = sht3x_sensor.read_temp_humd()
    # measure_data = [22.9759, 73.8277]
    # The default decimal place is 4 digits
    temp = measure_data[0]
    humd = measure_data[1]
    
    # 擦屏幕
    oled.fill(0)
    
    # 写中文（字符、屏幕x坐标、屏幕y坐标）
    # 注意，每个坐标=16*16的像素（128*64的屏幕横着只能放8个字,竖着只能放4个字）
    oled.chinese('温度：',1,1)
    oled.chinese('湿度：',1,4)
    oled.chinese(str(temp),4,1)
    oled.chinese(str(humd),4,4)
    # 展示
    oled.show()

    print('Temp:', temp)
    # Temp: 22.9759
    print('Humd:', humd)
    # Humd: 73.8277
    time.sleep(3)