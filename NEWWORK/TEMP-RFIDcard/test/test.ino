/**
 * ============================================================================
 * DFPlayer Mini 独立测试程序（增强版）
 * 适用开发板：ESP32-S3 (ESP32S3 Dev Module)
 * 
 * 功能：
 *   1. 初始化 DFPlayer Mini，音量设为 20
 *   2. 短按 BOOT 按键（GPIO 0）切换播放/停止
 *   3. 播放时自动顺序播放 SD 卡根目录下的 0001.mp3、0002.mp3...（循环）
 *   4. 稳健的 BUSY 检测，避免因信号抖动提前停止
 *   5. 完整打印 DFPlayer 返回的错误、播放完成等信息
 * 
 * 硬件连接建议：
 *   DFPlayer TX  → ESP32-S3 GPIO 18 (RX2)
 *   DFPlayer RX  → ESP32-S3 GPIO 17 (TX2)
 *   DFPlayer BUSY → ESP32-S3 GPIO 21 (输入)
 *   DFPlayer VCC  → 独立 5V 电源（切勿仅靠开发板5V引脚！）
 *   DFPlayer GND  → 与 ESP32-S3 共地
 *   BOOT 按键     → GPIO 0（开发板自带，按下为低电平）
 * 
 * 重要提示：
 *   - DFPlayer 峰值电流 >200mA，请务必使用**外部5V电源**为其供电，
 *     仅接开发板5V引脚极易导致模块复位、播放中断。
 *   - SD卡必须格式化为 FAT16/FAT32，文件命名为 0001.mp3、0002.mp3...
 *   - 如使用独立电源，请将 DFPlayer GND 与 ESP32 GND 相连。
 * ============================================================================
 */

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

// --- 引脚定义 ----------------------------------------------------------------
constexpr int kDfPlayerTx = 17;      // ESP32 TX2  -> DFPlayer RX
constexpr int kDfPlayerRx = 18;      // ESP32 RX2  -> DFPlayer TX
constexpr int kDfPlayerBusy = 21;    // DFPlayer BUSY 引脚（低电平播放）
constexpr int kButtonBoot = 0;       // BOOT 按键（按下低电平）

// --- 全局对象 ----------------------------------------------------------------
HardwareSerial dfplayerSerial(2);    // 使用 UART2
DFRobotDFPlayerMini g_dfplayer;

// --- 状态变量 ----------------------------------------------------------------
bool g_isPlaying = false;           // 当前是否处于播放状态
int g_currentTrack = 1;             // 当前播放的曲目编号（1 = 0001.mp3）
unsigned long g_ignoreBusyUntil = 0; // 在此时间戳之前忽略 BUSY 检测
const int VOLUME = 20;             // 固定音量（0~30）

// 用于 BUSY 防抖：连续检测到 BUSY 高电平才认为播放结束
const int kBusyDebounceCount = 5;   // 需要连续检测到5次高电平
int g_busyHighCounter = 0;

// --- 函数声明 ----------------------------------------------------------------
bool InitDFPlayer();
void handleButton();
void playTrack(int trackNumber);
void playNextTrack();
void printDFPlayerFeedback();

// =============================================================================
// 初始化
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== DFPlayer Mini 独立测试程序（增强版）===");
    Serial.println("请确保 DFPlayer 使用独立 5V 供电，并与 ESP32 共地！");
    Serial.println("--------------------------------------------");

    // 初始化 DFPlayer
    if (!InitDFPlayer()) {
        Serial.println("【错误】DFPlayer 初始化失败！");
        Serial.println("可能原因：");
        Serial.println("  - 供电不足（请使用外部5V电源）");
        Serial.println("  - 串口接线错误（TX→RX, RX→TX）");
        Serial.println("  - 模块损坏或SD卡未插入");
        while (1) {
            delay(100); // 停止运行
        }
    }
    Serial.println("【成功】DFPlayer 初始化成功");

    // 设置音量和输出设备
    g_dfplayer.volume(VOLUME);
    g_dfplayer.EQ(DFPLAYER_EQ_NORMAL);
    g_dfplayer.outputDevice(DFPLAYER_DEVICE_SD);
    Serial.printf("音量设置为: %d\n", VOLUME);

    // 按键引脚配置（输入上拉）
    pinMode(kButtonBoot, INPUT_PULLUP);
    // BUSY 引脚配置（输入）
    pinMode(kDfPlayerBusy, INPUT);

    Serial.println("\n系统就绪！");
    Serial.println("操作说明：短按 BOOT 按键 -> 切换播放/停止");
    Serial.println("播放时自动循环播放 0001.mp3, 0002.mp3 ...");
    Serial.println("--------------------------------------------\n");
}

// =============================================================================
// 主循环
// =============================================================================
void loop() {
    // 1. 处理按键事件
    handleButton();

    // 2. 播放状态下的自动下一首（带防抖）
    if (g_isPlaying) {
        // 只在播放开始后稍晚一点再检测 BUSY（避免初始误判）
        if (millis() > g_ignoreBusyUntil) {
            bool busyPin = digitalRead(kDfPlayerBusy);
            
            // BUSY = HIGH 表示空闲（播放结束）
            if (busyPin == HIGH) {
                g_busyHighCounter++;
                // 需要连续多次检测到 HIGH 才认为是稳定结束状态
                if (g_busyHighCounter >= kBusyDebounceCount) {
                    Serial.printf("检测到 BUSY 持续高电平，曲目 %d 播放结束\n", g_currentTrack);
                    g_busyHighCounter = 0;       // 计数器清零
                    playNextTrack();              // 播放下一个
                }
            } else {
                // BUSY = LOW（播放中），重置计数器
                g_busyHighCounter = 0;
            }
        }
    } else {
        // 未播放时，确保计数器清零
        g_busyHighCounter = 0;
    }

    // 3. 处理 DFPlayer 返回的消息（错误、播放完成等）
    printDFPlayerFeedback();

    delay(10); // 小延时，降低 CPU 占用
}

// =============================================================================
// 初始化 DFPlayer Mini
// =============================================================================
bool InitDFPlayer() {
    Serial.println("正在初始化 DFPlayer Mini...");
    dfplayerSerial.begin(9600, SERIAL_8N1, kDfPlayerRx, kDfPlayerTx);
    delay(500);  // 等待模块启动

    if (!g_dfplayer.begin(dfplayerSerial, false, true)) {
        return false;
    }
    delay(1000);
    return true;
}

// =============================================================================
// 按键处理：检测短按，切换播放/停止
// =============================================================================
void handleButton() {
    static bool lastState = HIGH;
    static unsigned long pressStartTime = 0;
    static bool isPressing = false;
    const unsigned long debounceDelay = 50;     // 防抖延时
    const unsigned long longPressTime = 5000;   // 长按阈值（本测试暂不使用）

    bool currentState = digitalRead(kButtonBoot);

    // 检测下降沿（按键按下）
    if (lastState == HIGH && currentState == LOW) {
        pressStartTime = millis();
        isPressing = true;
    }

    // 检测上升沿（按键释放）
    if (lastState == LOW && currentState == HIGH) {
        if (isPressing) {
            unsigned long pressDuration = millis() - pressStartTime;
            if (pressDuration >= debounceDelay && pressDuration < longPressTime) {
                // --- 短按：切换播放/停止 ---
                if (!g_isPlaying) {
                    // 当前停止 -> 开始播放
                    g_isPlaying = true;
                    g_currentTrack = 1;   // 从头开始
                    Serial.println("\n【按键】开始顺序播放");
                    playTrack(g_currentTrack);
                } else {
                    // 当前播放 -> 停止
                    g_isPlaying = false;
                    g_dfplayer.stop();
                    Serial.println("\n【按键】停止播放");
                }
            }
        }
        isPressing = false;
    }

    lastState = currentState;
}

// =============================================================================
// 播放指定曲目（文件编号）
// =============================================================================
void playTrack(int trackNumber) {
    // 文件编号范围 1~99
    if (trackNumber < 1) trackNumber = 1;
    if (trackNumber > 99) trackNumber = 1;

    g_dfplayer.play(trackNumber);
    Serial.printf("▶ 播放曲目: %d (000%d.mp3)\n", trackNumber, trackNumber);

    // 记录当前正在播放的曲目编号
    g_currentTrack = trackNumber;

    // 设置 BUSY 忽略窗口：播放开始后 1 秒内不检测 BUSY
    // （防止播放启动瞬间 BUSY 信号不稳定）
    g_ignoreBusyUntil = millis() + 1000;
    
    // 重置 BUSY 防抖计数器
    g_busyHighCounter = 0;
}

// =============================================================================
// 播放下一个曲目（自动递增，循环 1~99）
// =============================================================================
void playNextTrack() {
    int nextTrack = g_currentTrack + 1;
    if (nextTrack > 99) nextTrack = 1;
    playTrack(nextTrack);
}

// =============================================================================
// 打印 DFPlayer 返回的反馈信息（调试用）
// =============================================================================
void printDFPlayerFeedback() {
    while (g_dfplayer.available()) {
        int type = g_dfplayer.readType();
        int value = g_dfplayer.read();

        switch (type) {
            case DFPlayerPlayFinished:
                Serial.printf("【DFPlayer】曲目 %d 播放完成（模块反馈）\n", value);
                // 注意：我们通过 BUSY 检测自动下一首，此处无需额外操作
                break;
            case DFPlayerError:
                Serial.printf("【DFPlayer 错误】错误码: %d - ", value);
                switch (value) {
                    case 0x01: Serial.println("无 SD 卡"); break;
                    case 0x02: Serial.println("文件错误（命名/格式）"); break;
                    case 0x03: Serial.println("设备未就绪"); break;
                    case 0x04: Serial.println("校验和错误"); break;
                    case 0x05: Serial.println("文件不存在"); break;
                    case 0x06: Serial.println("闪存错误"); break;
                    default:   Serial.println("未知错误"); break;
                }
                break;
            case DFPlayerFeedBack:
                // 模块对某些命令的确认反馈，可忽略
                break;
            default:
                // 其他类型（如 DFPlayerCardInserted, DFPlayerCardRemoved 等）
                // 可自行添加处理，这里只打印数值
                Serial.printf("【DFPlayer】类型=%d, 数值=%d\n", type, value);
                break;
        }
    }
}