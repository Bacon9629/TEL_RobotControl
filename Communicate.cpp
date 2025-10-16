#include <Arduino.h>
#include <JY901.h>
#include "Communicate.h"
#include "Tool.h"
#include "Parameter.h"

Communicate::Communicate(){}

Communicate::~Communicate(){};

void Communicate::reset_angle_offset(){
    first_angle = 0;
    first_angle = get_angle();
}

void Communicate::attach_jetson_nano_Serial(
        HardwareSerial *_jetson_nano_Serial,
        void (*_recieve_jetson_nano)(char dir_code, int speed, int target_angle, int servo0_angle, int servo1_angle),
        void (*_recieve_jetson_nano_action)(char mission_code)
    ){

    jetson_nano_Serial = _jetson_nano_Serial;
    (*jetson_nano_Serial).begin(_MY_BAUD_);

    recieve_jetson_nano = _recieve_jetson_nano;
    recieve_jetson_nano_action = _recieve_jetson_nano_action;
}

void Communicate::attach_control_mega_Serail(HardwareSerial *_control_mega_Serail){
    control_mega_Serail = _control_mega_Serail;
    (*control_mega_Serail).begin(_MY_BAUD_);
}

void Communicate::attach_jy61_Serial(
    HardwareSerial *jy61_Serail, 
    void (*recieve_jy61_now_angle)(int now_angle)
    ){
    // this->jy61_Serail = jy61_Serail;
    jy61_enable = true;

    (*jy61_Serail).begin(115200);
    JY901.attach(*jy61_Serail);

    first_angle = jy61_serial_read();

    this->recieve_jy61_now_angle = recieve_jy61_now_angle;
}

void Communicate::read_serial_buffer(){
    jetson_nano_serial_read();
    jy61_serial_read();
    // delay(_READ_DELAY_);
}

void Communicate::send_jetson_nano_mission(char mission_success){
    if (!jetson_nano_Serial){
        return;
    }
    (*jetson_nano_Serial).print(mission_success);
}

void Communicate::send_motor_mega(int now_angle, int target_angle, char dir_code, int speed){
    if (!control_mega_Serail){
        return;
    }

    if (now_angle > 360){
        now_angle -= 360;
    }else if(now_angle < 0){
        now_angle += 360;
    }

    if (target_angle > 360){
        target_angle -= 360;
    }else if(target_angle < 0){
        target_angle += 360;
    }

    char temp[8];
    sprintf(temp, "~%c%03d", dir_code, speed);
    // Serial.println(temp);
    (*control_mega_Serail).print(temp);

    char temp2[10];
    sprintf(temp2, "[%03d%03d", now_angle, target_angle);
    // Serial.println(temp2);
    (*control_mega_Serail).print(temp2);
};

void Communicate::jetson_nano_serial_read(){
    /*
    使用Serial1
    取得jetson nano傳過來的資料，資料分成兩部分，詳見編碼表
    1. <行動>
    2. [方向、速度、自走車目標角度、手臂馬達1、手臂馬達2]
    */

    // // test start
    // while((*jetson_nano_Serial).available()){
    //     Serial.write((*jetson_nano_Serial).read());
    // }
    // return;
    // // test end

    if (!jetson_nano_Serial){
        return;
    }

    if (!(*jetson_nano_Serial).available()){
        return;
    }

    char type = (*jetson_nano_Serial).read();

    switch (type)
    {
        case '<':{
            delay(1);
            char action_code = (*jetson_nano_Serial).read();
            if (recieve_jetson_nano_action){
                (*recieve_jetson_nano_action)(action_code);
            }
            break;
        }
        
        case '[':{
            delay(1);
            
            char dir_code = (*jetson_nano_Serial).read();
            int speed = tool.char2int(jetson_nano_Serial);
            int target_angle = tool.char2int(jetson_nano_Serial);
            int servo0 = tool.char2int(jetson_nano_Serial);
            int servo1 = tool.char2int(jetson_nano_Serial);

            if (target_angle > 180){
                target_angle -= 360;
            }else if(target_angle < 0){
                target_angle += 360;
            }
            // Serial.print("!: ");
            // Serial.println(tool.char2int(jetson_nano_Serial));
            // int servo1 = 90;

            // debug - on

            // char temp[50];
            // sprintf(temp, "%c, speed: %03d, target_angle: %03d, servo0: %03d, servo1: %03d", dir_code, speed, target_angle, servo0, servo1);
            // Serial.println(temp);

            // debug - off

            if (recieve_jetson_nano){
                (*recieve_jetson_nano)(dir_code, speed, target_angle, servo0, servo1);
            }
            break;

        }

        // default:{
        //     char temp[20];
        //     sprint(temp, "trash: %c");
        //     break;
        // }
    }
    
    // while((*jetson_nano_Serial).available()){
    //     (*jetson_nano_Serial).read();
    // }

};


int Communicate::jy61_serial_read(){
    if (recieve_jy61_now_angle){
        int now_angle = get_angle();
        (*recieve_jy61_now_angle)(now_angle);
        return now_angle;
    }
    return 0;

    // Serial.print("Angle:");
	// Serial.print(JY901.getRoll());
	// Serial.print(" ");
	// Serial.print(JY901.getPitch());
	// Serial.print(" ");
	// Serial.print(JY901.getYaw());
	// Serial.print("\n");
};

inline int Communicate::get_angle(){
    if (!jy61_enable){
        return 0;
    }
    JY901.receiveSerialData();
    int now_angle = (JY901.getYaw()) * -1;
    // int now_angle = (JY901.getYaw());
    now_angle = now_angle - first_angle;
    if (now_angle > 180){
        now_angle -= 360;
    }else if(now_angle < -180){
        now_angle += 360;
    }
    return now_angle;
}

void Communicate::set_jy61_enable(bool enable){
    jy61_enable = enable;
}