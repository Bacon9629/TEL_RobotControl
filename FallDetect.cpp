#include <Arduino.h>
#include "FallDetect.h"
#include "Parameter.h"
#include "pin.h"

FallDetect::FallDetect(HandServo _hand_servo): hand_servo(_hand_servo){}
FallDetect::~FallDetect(){}

void FallDetect::enable(bool active){
    /**
     * 是否啟動防落偵測
     * 
     */
    enable_active = active;
}

void FallDetect::sort(double *list, size_t size){
    for (int i = 0; i < size; i++) {
        int j = i;
        while (j > 0 && list[j - 1] > list[j]) {
        int temp = list[j];
        list[j] = list[j - 1];
        list[j - 1] = temp;
        j--;
        }
    }
}

/**
 * @brief 取得偵測器距離，若超過30則回傳100
 * 
 * @param pin 要偵測的感測器
 * @return int 偵測器偵測到的距離，超過30則輸出100
 */
int FallDetect::get_distance(byte pin){
    if (!enable_active) {
        return 0;
    }
    hand_servo.temp_detach();

    const size_t size = 300;  // 這裡改取值次數

    double history[size];
    double result = 0;

    for (size_t i=0;i<size;i++){
        double temp;
        // if (fallpin.rff == pin){
        //     temp = 10650.08 * pow(analogRead(pin),-0.935) - 10;
        // }else{
        //     temp = 13 * pow((analogRead(pin) * 0.0048828125), -1);
        // }
        temp = 13 * pow((analogRead(pin) * 0.0048828125), -1);
        

        // int sum = 0;
        // for (int i=0; i<100; i++){
        //     sum = sum + float(analogRead(pin));  
        // }
        // sensorVal = sum / 100;
        // sensorVolt = sensorVal * Vr / 1024;
        
        // distance = pow(sensorVolt*(1 / k1), 1 / k2);

        history[i] = temp;
    }
    // sort(history, size);
    // result = history[90];

    result = history[0];
    for (size_t i=0;i<size;i++){
        if (history[i] > result){
            result = history[i];
        }
    }

    // result += history[int(result/2)-1];
    // result += history[int(result/2)];
    // result += history[int(result/2)+1];
    // result /= 3;

    hand_servo.re_attach();
    
    // if (fallpin.rff == pin){
    //     if (result > 150){
    //         return 300;
    //     }
    // }else{
    //     if (result > 30){
    //         return 300;
    //     }
    // }
    if (result > 30){
        return 100;
    }

    return int(result);
}

bool FallDetect::get_is_arrive(byte pin){
    if (!enable_active) {
        return false;
    }

    // if (pin == fallpin.mfd){
    //     return get_distance(pin) <= 7;

    // }else if(pin == fallpin.mbd){
    //     return get_distance(pin) <= 7;

    // }

    return get_distance(pin) <= _FALL_SENSOR_DETECT_ARRIVE_DISTANCE_;
}

bool FallDetect::get_is_fall(byte pin){
    if (!enable_active) {
        return false;
    }

    return get_distance(pin) > _FALL_SENSOR_DETECT_FALL_DISTANCE_;
}

// bool FallDetect::get_is_fall(byte *pins, int amount){
//     if (!enable_active) {
//         return false;
//     }

//     for (int i=0;i<amount;++i){
//         if (get_is_fall(pins[i])){
//             return true;
//         }
//     }
//     return false;
// }

// bool FallDetect::get_is_fall(){
    // if (get_is_fall(fallpin.lmd)){
    //     return true;
    // } else if (get_is_fall(fallpin.mfd)){
    //     return true;
    // } else if (get_is_fall(fallpin.rmd)){
    //     return true;
    // } else if (get_is_fall(fallpin.mbd)){
    //     return true;
    // }
    // return false;
    // byte pins[] = {fallpin.lmd, fallpin.mfd, fallpin.rmd, fallpin.mbd};
    // return get_is_fall(pins, 4);
// }