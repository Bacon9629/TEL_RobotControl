
#include <Arduino.h>
#include <Servo.h>
#include "HandServo.h"
#include "Parameter.h"

HandServo::HandServo(){
};
HandServo::~HandServo(){};

void HandServo::attach(const byte servo_pin[2], const byte pump_pin, const byte relay_pin){
    servos[0].attach(servo_pin[0]);
    servos[1].attach(servo_pin[1]);
    this->servo_pin[0] = servo_pin[0];
    this->servo_pin[1] = servo_pin[1];
    this->pump_pin = pump_pin;
    this->relay_pin = relay_pin;
    pinMode(pump_pin, OUTPUT);
    pinMode(relay_pin, OUTPUT);
    
    active_pump(false);
}


void HandServo::temp_detach(){
    pinMode(pump_pin, INPUT);
    pinMode(relay_pin, INPUT);
}

void HandServo::re_attach(){
    pinMode(pump_pin, OUTPUT);
    pinMode(relay_pin, OUTPUT);
}

void HandServo::do_it(int angle_a, int angle_b, bool _pump_active){
    move_hand(angle_a, angle_b);
    active_pump(_pump_active);
}

void HandServo::do_it(int *angles, bool _pump_active){
    do_it(angles[0], angles[1], _pump_active);
}

inline void HandServo::move_hand(int *angles){
    move_hand(angles[0], angles[1]);
}

inline void HandServo::move_hand(int angle_a, int angle_b){
    servos[0].write(angle_a);
    servos[1].write(angle_b);
}

inline void HandServo::active_pump(bool active){

    if (pump_active == active){
        return;
    }
    // Serial.println(active ? "active : true" : "active : false");

    if (active){
        digitalWrite(pump_pin, 1);
        digitalWrite(relay_pin, 0);
        delay(_PUMP_DELAY_);
        delay(_PUMP_DELAY_);
    }else{
        digitalWrite(pump_pin, 0);
        digitalWrite(relay_pin, 1);
        delay(_PUMP_DELAY_);
        digitalWrite(relay_pin, 0);
    }


    pump_active = active;
    
    
};

inline bool HandServo::get_pump_active(){
    return pump_active;
};
