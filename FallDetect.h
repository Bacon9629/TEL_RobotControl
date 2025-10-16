#ifndef _FALL_DETECT_H_
#define _FALL_DETECT_H_

#include <Arduino.h>
#include "Parameter.h"
#include "pin.h"
#include "HandServo.h"

class FallDetect{
    private:
    FallPin fallpin;
    HandServo hand_servo;
    bool enable_active = true;

    float sensorVal = 0;
    float sensorVolt = 0;
    float Vr = 5.0;
    float sum = 0;
    float k1 = 16.7647563;
    float k2 =- 0.85803107;
    float distance = 0;

    void sort(double*, size_t);

    public:
    FallDetect(HandServo _hand_servo);
    ~FallDetect();

    void enable(bool active);  // 防落偵測是否啟動
    int get_distance(byte pin);  // 取得這個sensor的距離
    bool get_is_arrive(byte pin);  // 偵測下方是否有東西
    bool get_is_fall(byte pin);  // 取得此感測器是否偵測到要掉下來
    // bool get_is_fall(byte *pins, int amount);  // 取得這一串感測器是否偵測到要掉下來
    // bool get_is_fall();  // 偵測到要掉下來


};


#endif