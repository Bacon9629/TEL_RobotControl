/**
 * 名稱: Tool
 * 功能: 存放各種計算、轉換工具
 *
 */

#ifndef _TOOL_H_
#define _TOOL_H_

#include <Arduino.h>
// #include <USBAPI.h>

class Tool
{

public:
    Tool();
    ~Tool();
    int char2int(char *data);                 // 輸入三個"數字"字元，把輸入的三個數字轉成三位數int輸出
    int char2int(HardwareSerial *Serial_temp); // 自動從Serial讀取三個字元，並將其轉為三位數int輸出
};

#endif