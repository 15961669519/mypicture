from time import sleep
from sht3x import SHT3x_Sensor
import time

# 定义传感器对象(sda\scl为引脚pin)
sht3x_sensor = SHT3x_Sensor(freq=100000, sdapin=12, sclpin=14)

# 一个死循环
while(True):
    # 读取数据
    measure_data = sht3x_sensor.read_temp_humd()
    # measure_data = [22.9759, 73.8277]

    temp = measure_data[0]
    humd = measure_data[1]

    # 打印输出
    print('Temp:', temp)
    # Temp: 22.9759
    print('Humd:', humd)
    # Humd: 73.8277

    # 等待3秒后，进入下一次循环
    time.sleep(3)