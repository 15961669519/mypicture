#pragma once



    class PCA9685 {
    public:
        PCA9685(int i2cAddress = 0x40);
        ~PCA9685();

        void begin();
        void setPWMFreq(int freq);
        void setPWM(int index, int on, int off);
        void sleep();
        void wakeup();

    private:
        void* obj_;
    };

extern PCA9685 pwm1 ;