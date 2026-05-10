/**
 * 该代码可运行在ESP32、ESP8266上，不过后续我们大多数需要用到SD卡的作品，都会优先使用ESP32
 * ESP8266的SD卡驱动库代码结构稍微有点乱，ESP32则好的多。
 * 使用不同的单片机，仅需要修改 chipSelect 的引脚名称
 *  我的默认设置：8266使用D1、ESP32使用5
 *  
 *  该代码仅用于测试连接状态、读写成功性
 *  
 *    ESP8266
 *      3V3 => 3V3
 *      GND => GND
 *      CS  => D1
 *      MOSI=> D7
 *      MISO=> D6
 *      SCK => D5
 *    ESP32
 *      3V3 => 3V3
 *      GND => GND
 *      CS  => 5
 *      MOSI=> 23
 *      MISO=> 19
 *      SCK => 1803-3
 *  使用方法：
 *    烧写代码并正确连线后，打开串口监视器
 *    选择，波特率115200，不使用结束符
 *    命令：
 *      ls 列出所有文件
 *      ls 文件夹 列出指定文件夹的内容
 *      write /文件名 内容 向指定文件写入内容
 *      read /文件名   读取指定文件的内容
 *      rm /文件名   删除文件
 *      mkdir /文件夹  创建文件夹
 *      rmdir /文件夹  删除文件夹（文件夹必须是空的才能删除）
 */
#include "SPI.h"
#include "SD.h"

const int chipSelect = D8;  //esp8266
//const int chipSelect = 5; //esp32
File dataFile;

void writeSdFile(String FileName, String str, boolean isln);
void readSdFile(String FileName);
void removeSdFile(String FileName);
void listSdFile(File dir, int numTabs);
void mkSdDir(String directoryName);
void rmSdDir(String directoryName);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  Serial.println("start system ..");

  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    while (1);
  }
}


String cmd;
String param;
String param2;

void loop() {
  if (Serial.available() > 0) {

    String str = Serial.readStringUntil('\r');
    Serial.flush();
    int pos = str.indexOf(32);  //the ascii code of the space
    if (pos == -1) {
      cmd = str;
      param = "";
    } else {
      cmd = str.substring(0, pos);
      param = str.substring(pos + 1, str.length()); //ignore space

      //还有参数2
      pos = param.indexOf(32);
      if ( pos > -1 ) {
        param2 = param.substring(pos + 1, param.length()); //ignore space
        param = param.substring(0, pos); //从param中把param2 的部分删除掉
      }
    }
    Serial.print(cmd);
    Serial.print(":");
    Serial.print(param);
    Serial.print(":");
    Serial.println(param2);

    //读取文件内容
    if (cmd == "read") {
      if (param.length() < 1) {
        Serial.println("please provide a filename");
      } else {
        readSdFile(param);
      }
      return;
    }
    //写入文件
    if (cmd == "write") {
      if (param.length() < 1 || param2.length() < 1) {
        Serial.println("please provide a filename");
      } else {
        writeSdFile(param, param2, true);
      }
      return;
    }

    //删除文件
    if (cmd == "rm") {
      if (param.length() < 1) {
        Serial.println("please provide a filename");
      } else {
        removeSdFile(param);
      }
      return;
    }

    //列出文件列表
    if (cmd == "ls") {
      if (param.length() < 1) {
        listSdFile(SD.open("/"), 0);
      } else {
        listSdFile(SD.open(param), 0);
      }
      return;
    }
    //创建文件夹
    if (cmd == "mkdir") {
      if (param.length() < 1) {
        Serial.println("please provide a directory name");
      } else {
        mkSdDir(param);
      }
      return;
    }

    //删除文件夹
    if (cmd == "rmdir") {
      if (param.length() < 1) {
        Serial.println("please provide a directory name");
      } else {
        rmSdDir(param);
      }
      return;
    }
  }
}

void writeSdFile(String FileName, String str, boolean isln = true) {
  dataFile = SD.open(FileName, FILE_WRITE);
  if (dataFile) {
    if (isln) {
      dataFile.println(str);
    } else {
      dataFile.print(str);
    }
    dataFile.close();
  } else {
    Serial.println("the file does not exist!");

  }
}

/**
   逐行读取文件
*/
void readSdFile(String FileName) {
  String line = "";
  dataFile = SD.open(FileName, FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      char ch = dataFile.read();
      line = line + ch;
      if (ch == '\r') {
        continue;
      }
      if (ch == '\n') {
        Serial.print(line);
        line = "";
      }
    }
    dataFile.close();
    Serial.println("read end");
  } else {
    Serial.println("the file does not exist!");
  }
}

/**
   删除文件
*/
void removeSdFile(String FileName) {
  if (SD.remove(FileName) == 1) {
    Serial.println("file:" + FileName + " is removed.");
  } else {
    Serial.println("the file does not exist!");
  }
}

/**
   读取文件列表
*/
void listSdFile(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      break;
    }
    //过滤掉隐藏目录
    if (String(entry.name()).substring(0, 1) == ".") {
      continue;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print("name: ");
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listSdFile(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
//创建文件夹
void mkSdDir(String directoryName) {
  int r = SD.mkdir(directoryName);
  if (r == 1) {
    Serial.println("directory name:" + directoryName + " is created.");
  } else {
    Serial.println("error!");
  }
}
//删除文件夹
void rmSdDir(String directoryName) {
  int r = SD.rmdir(directoryName);
  Serial.println(r);
  if (r == 1) {
    Serial.println("directory name:" + directoryName + " is removed.");
  } else {
    Serial.println("error! maybe the directory is not empty.");
  }
}
