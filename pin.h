#ifndef _PIN_H_
#define _PIN_H_


#include <Arduino.h>

const byte hand_servos_pin[2] = {6, 7};
const byte pump_pin = 4;
const byte relay_pin = 3;

const byte storage_hand_pin = 5;

class FallPin{
    public:
    const byte lff = A5;  // 左 前 向斜偵測
    // const byte mff = 0;  // 中 前 向前偵測
    const byte mfd = A8;  // 中 前 向下偵測
    const byte rff = A4;  // 右 前 向斜偵測
    const byte lmf = A10;  // 左 中 向前偵測
    const byte lmd = A13;  // 左 中 向下偵測
    const byte rmf = A6;  // 右 中 向前偵測
    const byte rmd = A9;  // 右 中 向下偵測
    const byte mbd = A12;  // 中 後 向下偵測
    const size_t size = 8;
};
#endif