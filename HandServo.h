
#ifndef _HAND_MOTOR_H_
#define _HAND_MOTOR_H_

#include <Arduino.h>
#include <Servo.h>

class HandServo
{
private:
    int last_angle[2] = {-1, -1};
    byte servo_pin[2];
    bool pump_active = false;
    byte relay_pin = false;
    Servo servos[2];
    byte pump_pin;


public:
    HandServo();
    ~HandServo();

    void attach(const byte servo_pin[2], const byte pump_pin, const byte relay_pin);

    void temp_detach();
    void re_attach();

    void do_it(int angle_0, int angle_1, bool pump_active);
    void do_it(int *angles, bool pump_active);
    inline void move_hand(int angle_a, int angle_b);
    inline void move_hand(int *angles);
    inline void active_pump(bool active);
    inline bool get_pump_active();
};


#endif