/**
 * 
 * 名稱: Tool.cpp
 * 功能: 存放各種計算、轉換工具
 * 
*/

#include "Tool.h"
#include <Arduino.h>
// #include <USBAPI.h>

Tool::Tool(){}

Tool::~Tool(){}

int Tool::char2int(char *data){
    char temp[3] = {
        data[0] - '0',
        data[1] - '0',
        data[2] - '0'
    };
    Serial.print(int(temp[0]) * 100 + temp[1] * 10 + temp[2]);
    return int(temp[0]) * 100 + int(temp[1]) * 10 + int(temp[2]);
}

int Tool::char2int(HardwareSerial *Serial_temp){
    char data[3] = {
        Serial_temp->read(),
        Serial_temp->read(),
        Serial_temp->read()
        };
    return Tool::char2int(data);
}
