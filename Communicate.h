
#ifndef _Communication_h_
#define _Communication_h_

#include <Arduino.h>
#include "Tool.h"

class Communicate
{
private:
    HardwareSerial *jetson_nano_Serial;
    HardwareSerial *control_mega_Serail;
    HardwareSerial *jy61_Serail;

    int first_angle = 0;
    Tool tool;
    bool jy61_enable = false;

    void (*recieve_jetson_nano)(char dir_code, int speed, int target_angle, int servo0_angle, int servo1_angle);
    void (*recieve_jetson_nano_action)(char mission_code);
    void (*recieve_jy61_now_angle)(int now_angle);

    void jetson_nano_serial_read();
    int jy61_serial_read();

public:
    Communicate();
    ~Communicate();

    void reset_angle_offset();

    void attach_jetson_nano_Serial(
        HardwareSerial *jetson_nano_Serial,
        void (*recieve_jetson_nano)(char dir_code, int speed, int target_angle, int servo0_angle, int servo1_angle),
        void (*recieve_jetson_nano_action)(char mission_code)
        );
    void attach_control_mega_Serail(HardwareSerial *control_mega_Serail);
    void attach_jy61_Serial(
        HardwareSerial *jy61_Serail, 
        void (*recieve_jy61_now_angle)(int now_angle)
        );

    void read_serial_buffer();  // 放進loop迴圈內一直跑

    void send_jetson_nano_mission(char mission_success);
    void send_motor_mega(int now_angle, int target_angle, char dir_code, int speed);

    inline int get_angle();

    void set_jy61_enable(bool);


};

#endif
