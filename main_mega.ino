#include <Arduino.h>
#include "Tool.h"
#include "Communicate.h"
#include "HandServo.h"
#include "pin.h"
#include "FallDetect.h"

int speed_range[3] = {25, 80, 100};

char move_dir_code = ' ';  // 移動方向編碼、無段的那種，詳見編碼表
char action_code = ' ';  // jetson nano to mega 行動指令編碼
int move_speed = 0;
int target_angle = 0;
int hand_motor_angle[2] = {90, 90};
bool pump_active = false;
int now_angle = 0;

Tool tool;
Communicate commu;
HandServo hand_servo;
FallPin fallpin;
FallDetect falldetect(hand_servo);

Servo storage_hand_servo;

// 監聽函式
void recieve_jetson_nano(char dir_code, int speed, int target_angle, int servo0_angle, int servo1_angle);
void recieve_jetson_nano_action(char mission_code);  // 接收到jetson nano任務資料時會執行的
void recieve_jy61_now_angle(int now_angle_);  // jy61 接收到角度時會執行的函式

void storage_out(bool let_out);  // 排料，let_out = true：排
void first_go_front_side();  // 第一關定位到起點方框前
void mission_f();  // 第一關定位到起點方框後


void setup(){

    Serial.begin(115200);

    commu.attach_jetson_nano_Serial(&Serial2, &recieve_jetson_nano, &recieve_jetson_nano_action);
    commu.attach_control_mega_Serail(&Serial1);
    commu.attach_jy61_Serial(&Serial3, &recieve_jy61_now_angle);

    hand_servo.attach(hand_servos_pin, pump_pin, relay_pin);
    hand_servo.move_hand(115, 110);
    hand_servo.active_pump(false);

    storage_hand_servo.attach(storage_hand_pin);

    delay(300);
    falldetect.enable(true);    
    commu.reset_angle_offset();
    commu.set_jy61_enable(true);


    Serial.println("gogo");
    move('p', 100, 10);

    storage_out(false);

}

void loop(){

    static bool check = false;
    if (check){
        char a[50];
        sprintf(a, "angle: %03d, rff: %03d, mfd: %03d, lff: %03d"
            , commu.get_angle(), falldetect.get_distance(fallpin.rff),
            falldetect.get_distance(fallpin.mfd), falldetect.get_distance(fallpin.lff));
        Serial.println(a);
    }
    if (Serial.available()){
        char temp = Serial.read();
        if (temp == 'a'){
            first_go_front_side();
        }else if(temp == 'b'){
            first_re_go_front_side();
        }else if(temp == 'c'){
            first_go_right_side();
        }else if(temp == 'd'){
            first_re_go_right_side();
        }else if(temp == 'e'){
            first_go_back_side();
        }else if(temp == 'f'){
            first_re_go_back_side();
        }else if(temp == 'z'){
            target_angle = -135;
            toward_target_angle();
        }else if (temp == 'g'){
            check = !check;
        }else if(temp == 'h'){
            first_go_target_from_front();
        }else if(temp == 'i'){
            first_go_target_from_side();
        }else if(temp == 'j'){
            first_go_target_from_back();
        }else if(temp == 'k'){
            second_mission();
        }else if (temp == 'l'){
            third_mission();
        }
    }

    // move('w', speed_range[0], 10);
    
    // goto_edge(fallpin.lmd, 'a', speed_range[0]);
    // move('d', speed_range[0], 100);
    // delay(1000);
    // goto_until_detect(fallpin.mfd, 'w', speed_range[0], 12);
    // delay(1000000);

    // hand_servo.move_hand(180, 30);
    // hand_servo.move_hand(155, 60);
    // hand_servo.move_hand(130, 90);
    // Serial.println(falldetect.get_distance(fallpin.mfd));


    commu.read_serial_buffer();



}

void recieve_jetson_nano(char dir_code, int speed, int __target_angle, int servo0_angle, int servo1_angle){
    int _now_angle = now_angle;
    int _target_angle = __target_angle;

    now_angle = commu.get_angle();
    _now_angle = now_angle;
    _target_angle = __target_angle;
    if (_target_angle == 180){
        _target_angle = 0;
        if (_now_angle > 0){
            _now_angle = _now_angle - 180;
        }else{
            _now_angle = (_now_angle + 180);
        }
    }
    
    commu.send_motor_mega(_now_angle, _target_angle, dir_code, speed);

    hand_servo.move_hand(servo0_angle, servo1_angle);

    move_dir_code = dir_code;
    move_speed = speed;
    target_angle = __target_angle;
    hand_motor_angle[0] = servo0_angle;
    hand_motor_angle[1] = servo1_angle;

    char temp[30];
    sprintf(temp, "%c, %03d, %03d, %03d, %03d", dir_code, speed, _target_angle, servo0_angle, servo1_angle);
    Serial.println(temp);
    Serial.print("now angle ");
    Serial.println(now_angle);
}

void recieve_jetson_nano_action(char mission_code){
    /**
     * 接收任務指令，做完後須回傳做得如何
     * commu.send_jetson_nano_mission(bool mission_success)
     */

    char good_done = 'a';
    Serial.print("action code: ");
    Serial.println(mission_code);

    switch (mission_code)
    {
        case 'a':{
            // 吸盤吸
            hand_servo.active_pump(true);
            // Serial.println("A");
            break;
        }
        
        case 'b':{
            // 吸盤放
            hand_servo.active_pump(false);
            // Serial.println("B");
            break;
        }
        
        case 'c':{
            // 2F倒料 - on
            storage_out(true);
            break;
        }
        
        case 'd':{
            // 2F倒料 - off
            storage_out(false);
            break;
        }
        
        case 'e':{
            // 第一關定位到起點方框前
            
            first_go_front_side();
            break;
        }
        
        case 'f':{
            // 第一關從側面定位到起點方框後
            first_go_back_side();
            break;
        }
        
        case 'g':{
            // 第一關從起點方框 "前" 到終點方框，倒料後，正面面向終點方框
            first_go_target_from_front();
            break;
        }
        
        case 'h':{
            // 第一關從起點方框 "後" 到終點方框，倒料後，正面面向終點方框
            first_go_target_from_back();
            break;
        }
        
        case 'i':{
            // 第一關重新定位到終點方框
            first_from_start_line_go_target_front();
            break;
        }

        case 'j':{
            // 從第一關出發到第二關，並自動通過第二三關
            first_from_start_directly_go_second_and_third();
            break;
        }
        
        case 'k':{
            // 從第二關重新開始
            second_mission();
            third_mission();
            break;
        }
        
        case 'l':{
            // 從第三關重新開始
            third_mission();
            break;
        }
        
        case 'm':{
            // 回傳前面是否偵測到
            good_done = falldetect.get_is_arrive(fallpin.mfd) ? 'b' : 'a';

            break;
        }
        
        case 'n':{
            // 更新初始角度，會依此角度做回正的0度角
            commu.reset_angle_offset();
            target_angle = 0;
            break;
        }
        
        case 'o':{
            // 第一關重新定位到方框前
            first_re_go_front_side();
            break;
        }
        
        case 'p':{
            // 第一關重新定位到方框右側
            first_re_go_right_side();

            break;
        }
        
        case 'q':{
            // 第一關重新定位到方框後側
            first_re_go_back_side();
            break;
        }

        case 'r':{
            // 回傳掉落感測器是否偵測到
            if (falldetect.get_is_fall(fallpin.lmd) || falldetect.get_is_fall(fallpin.rmd)){
                good_done = false;
            }
            break;
        }
        
        case 's':{
            // 關閉回正
            commu.set_jy61_enable(false);
            break;
        }
        
        case 't':{
            // 開啟回正
            commu.set_jy61_enable(true);
            break;
        }

        case 'u':{
            // 第一關定位到方框測
            first_go_right_side();
            break;
        }

        case 'v':{
            // 第一關從方框側到終點方框，倒料後，正面面向終點方框
            first_go_target_from_side();
            break;
        }

        case 'w':{
            // 從第一關終點方框前往第二關前進、並破完二三關
            first_go_sencond_start_from_target_front();
            second_mission();
            third_mission();
            break;
        }

        case 'z':{
            // 回正
            toward_target_angle();
            break;
        }
        
        
        default:
            break;
    }

    Serial.print("return code: ");
    Serial.println(good_done);
    commu.send_jetson_nano_mission(good_done);

}

void recieve_jy61_now_angle(int now_angle_){
    now_angle = now_angle_;
}

/**
 * @brief 排料
 * 
 * @param let_out true:排；false:不排
 */
void storage_out(bool let_out){
    // 排料
    storage_hand_servo.write(let_out ? 180 : 78);
}

/**
 * @brief 
 * 第一關定位到起點方框前
 */
void first_go_front_side(){

    // 快速左前
    move('q', speed_range[2], 1500);
    
    // 慢速左前，直到到邊緣
    goto_edge(fallpin.lmd, 'q', speed_range[0]);

    move('d', speed_range[1], 500);

    goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);

}

/**
 * @brief 第一關「重新」定位到方框前
 * 
 */
void first_re_go_front_side(){
    // 慢速左，直到到邊緣
    move('s', speed_range[1], 300);
    target_angle = 0;
    toward_target_angle();
    goto_keep_distance(fallpin.mfd, 'w', 's', 10);
    goto_edge(fallpin.lmd, 'a', speed_range[0]);
    move('d', speed_range[1], 500);
    // goto_until_detect(fallpin.mfd, 'w', speed_range[0], 15);
    goto_keep_distance(fallpin.mfd, 'w', 's', 10);
}

/**
 * @brief 第一關從方框前定位到方框側邊
 * 
 */
void first_go_right_side(){
    target_angle = 0;
    toward_target_angle();
    goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);
    goto_until_no_detect(fallpin.mfd, 'd', speed_range[1], 20);
    move('d', speed_range[1], 500);
    move('w', speed_range[1], 650);
    move('p', speed_range[1], 500);
    target_angle = -90;
    toward_target_angle();
    move('d', speed_range[1], 750);
    goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);


}

/**
 * @brief 第一關在方框右側時，呼叫此重新回到方框側邊位置
 * 
 */
void first_re_go_right_side(){
    target_angle = -90;
    toward_target_angle();
    goto_keep_distance(fallpin.mfd, 'w', 's', 10);
    // goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);
    goto_until_no_detect(fallpin.mfd, 'a', speed_range[1], 30);
    goto_until_detect(fallpin.mfd, 'd', speed_range[1], 30);
    move('d', speed_range[1], 650);
    goto_keep_distance(fallpin.mfd, 'w', 's', 10);
    // goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);
    // toward_target_angle();

}

/**
 * @brief 第一關從右側定位到方框後側
 * 
 */
void first_go_back_side(){
    target_angle = -90;
    toward_target_angle();
    goto_until_no_detect(fallpin.mfd, 'd', speed_range[1], 25);
    move('d', speed_range[1], 2000);
    target_angle = 180;
    toward_target_angle();
    move('d', speed_range[1], 1500);
    goto_edge(fallpin.rmd, 'd', speed_range[0]);
    move('a', speed_range[1], 300);
    goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);

}


/**
 * @brief 第一關在方框後移動後重新定位回方框後側
 * 
 */
void first_re_go_back_side(){
    target_angle = 180;
    toward_target_angle();
    goto_edge(fallpin.rmd, 'd', speed_range[0]);
    move('a', speed_range[1], 300);
    move('s', speed_range[1], 500);
    goto_until_detect(fallpin.mfd, 'w', speed_range[0], 10);
    // goto_keep_distance(fallpin.mfd, 'w', 's', 10);
}

/**
 * @brief 第一關從方框前往終點移動
 * 
 */
void first_go_target_from_front(){
    move('s', speed_range[2], 1500);
    move('e', speed_range[2], 2500);
    goto_edge(fallpin.rmd, 'e', speed_range[0]);

    move('a', speed_range[1], 450);
    target_angle = 180;
    toward_target_angle();
    goto_until_detect(fallpin.mbd, 's', speed_range[1], 8);
    first_shake_to_out_storage();
    // first_go_sencond_from_target_front();

}

/**
 * @brief 第一關從方框側往終點移動
 * 
 */
void first_go_target_from_side(){
    target_angle = 180;
    move('s', speed_range[1], 1000);
    // target_angle = 0;
    toward_target_angle();
    goto_edge(fallpin.lmd, 'a', speed_range[0]);
    move('d', speed_range[1], 450);

    // move('a', speed_range[1], 500);
    // target_angle = 180;
    // toward_target_angle();
    goto_until_detect(fallpin.mbd, 's', speed_range[0], 8);
    first_shake_to_out_storage();
    // first_go_sencond_from_target_front();
}


/**
 * @brief 從方框後面前往目標方框，這時候target_angle = 180;
 * 
 */
void first_go_target_from_back(){
    target_angle = 180;
    move('a', speed_range[1], 2000);
    goto_edge(fallpin.lmd, 'a', speed_range[0]);


    move('d', speed_range[1], 450);
    toward_target_angle();
    goto_until_detect(fallpin.mbd, 's', speed_range[0], 8);
    first_shake_to_out_storage();
    // first_go_sencond_from_target_front();
}

/**
 * @brief 前後甩動，把方塊甩下去，還有把倉庫開啟與關閉
 * 
 */
void first_shake_to_out_storage(){

    move('s', speed_range[0], 1000);
    storage_out(true);
    delay(3000);

    for(int i=0;i<3;i++){
        move('w', speed_range[2], 150);
        move('p', 0, 100);
        move('s', speed_range[2], 300);
        move('p', 0, 100);
    }

    delay(3000);
    storage_out(false);

    move('w', speed_range[2], 300);
    move('p', speed_range[2], 300);

    target_angle = 0;
    toward_target_angle();

    goto_keep_distance(fallpin.mfd, 'w', 's', 10);

}

/**
 * @brief 從目標方框前到第二關
 * 
 */
void first_go_sencond_start_from_target_front(){
    // goto_until_no_detect(fallpin.mbd, 'd', speed_range[1], 20);
    // move('d', speed_range[2], 550);
    // target_angle = 0;
    // toward_target_angle();
    // move('w', speed_range[2], 3000);
    // move('p', 0, 100);

    goto_until_detect(fallpin.mfd, 'w', speed_range[1], 15);
    goto_until_no_detect(fallpin.mfd, 'a', speed_range[1], 25);
    move('a', speed_range[1], 800);
    move('w', speed_range[1], 2000);
    move('p', speed_range[1], 200);

}
    
/**
 * @brief 第一關從頭回到終點方框前
 * 
 */
void first_from_start_line_go_target_front(){
    move('w', speed_range[2], 1000);
    move('e', speed_range[1], 1500);
    goto_edge(fallpin.rmd, 'd', speed_range[0]);
    move('a', speed_range[1], 500);
    goto_keep_distance(fallpin.mfd, 'w', 's', 10);
}

/**
 * @brief 從起點直接去破第二關
 * 
 */
void first_from_start_directly_go_second_and_third(){
    first_from_start_line_go_target_front();
    first_go_sencond_start_from_target_front();
    second_mission();
    third_mission();
}

/**
 * @brief 第二關程式
 * 
 */
void second_mission(){
    // Serial.println("second mission");
    move('d', speed_range[1], 1000);
    goto_edge(fallpin.rmd, 'd', speed_range[0]);
    move('a', speed_range[1], 650);
    // toward_target_angle();
    goto_until_detect(fallpin.rff, 'w',speed_range[1], 25);
    // move('a', speed_range[1], 1000);
    move('w', speed_range[1], 300);
    goto_until_no_detect(fallpin.lff, 'w', speed_range[2], 30);

    // stay_center();
    // move('w', speed_range[1], 1000);
    // move('p', 0, 10);
    // return;

    // 前往下一個區域
    // 往前直到真測到左方有東西，往後一點在往左走，直到沒偵測到木條後往前走
    move('w', speed_range[1], 1000);
    move('a', speed_range[2], 1650);
    goto_edge(fallpin.lmd, 'a', speed_range[0]);
    move('d', speed_range[1], 750);
    move('w', speed_range[1], 1000);
    goto_until_no_detect(fallpin.lff, 'w', speed_range[2], 30);

    // stay_center();

    // 前往下一個區域
    // 往前直到真測到右方有東西，往後一點在往右走，直到沒偵測到木條後往前走
    move('w', speed_range[1], 850);
    goto_until_detect(fallpin.lmf, 'd', speed_range[0], 27);  // 平移植到映前偵測碰到東西move('a', speed_range[1], 300);
    goto_until_no_detect(fallpin.lmf, 'd', speed_range[0], 27);
    move('d', speed_range[1], 200);
    move('w', speed_range[1], 1000);
    goto_until_no_detect(fallpin.lff, 'w', speed_range[2], 30);
    // stay_center();
    // move('p', 0, 10);

    // 通過第二關
    move('w', speed_range[2], 1500);
    move('p', speed_range[1], 10);



}

/**
 * @brief 第二關的，讓車子保持在兩木條的中間
 * 
 */
void stay_center(){
    int left_distance = falldetect.get_distance(fallpin.lff);
    int right_distance = falldetect.get_distance(fallpin.rff);
    int error_distance = abs(left_distance - right_distance);

    while(left_distance <  30 && right_distance < 30){
        Serial.print("inin: ");
        Serial.println(target_angle);

        target_angle = (right_distance - left_distance) * 2;
        Serial.println(target_angle);
        move('w', speed_range[1], 50);

        left_distance = falldetect.get_distance(fallpin.lff);
        right_distance = falldetect.get_distance(fallpin.rff);
        error_distance = abs(left_distance - right_distance);
        now_angle = commu.get_angle();

    }

    target_angle = 0;
    
}

/**
 * @brief 第三關往終點前進
 * 
 */
void third_mission(){
    target_angle = 0;
    move('d', speed_range[1], 350);
    goto_edge(fallpin.rmd, 'd', speed_range[0]);
    move('a', speed_range[1], 400);
    toward_target_angle();
    goto_until_detect(fallpin.lmf, 'w', speed_range[2], 28);

    // move('d', speed_range[1], 1500);
    // goto_edge(fallpin.rmd, 'd', speed_range[0]);
    // move('a', speed_range[1], 700);

    move('s', speed_range[1], 900);
    target_angle = 83;
    toward_target_angle_2();
    // target_angle = 83;
    // move('d', speed_range[2], 500);
    goto_until_detect(fallpin.lff, 'a', speed_range[0], 13);
    goto_until_no_detect(fallpin.lff, 's', speed_range[0], 28);
    move('s', speed_range[1], 250);

    goto_until_detect(fallpin.rmf, 'a', speed_range[0], 28);
    move('a', speed_range[1], 350);

    // target_angle = 80;
    // goto_until_detect(fallpin.lff, 'w', speed_range[1], 15);
    move('w', speed_range[1], 1000);
    move('a', speed_range[1], 5000);
    move('p', speed_range[0], 20);
}

/**
 * @brief 在main_mega程式內才能呼叫的移動函式
 * 
 * @param direction 移動指令編碼
 * @param speed 速度
 * @param delay_time 要移動多久時間
 */
void move(char direction, int speed, unsigned long delay_time){
    int _now_angle = commu.get_angle();
    int _target_angle = target_angle;

    if (_target_angle == 180){
        _target_angle = 0;
        if (_now_angle > 0){
            _now_angle = _now_angle - 180;
        }else{
            _now_angle = (_now_angle + 180);
        }
    }

    commu.send_motor_mega(_now_angle, _target_angle, direction, speed);
    delay(10);
    for (size_t i = 0; i < delay_time; i += 20){
        
        int _now_angle = commu.get_angle();
        int _target_angle = target_angle;

        if (_target_angle == 180){
            _target_angle = 0;
            if (_now_angle > 0){
                _now_angle = _now_angle - 180;
            }else{
                _now_angle = (_now_angle + 180);
            }
        }
        commu.send_motor_mega(_now_angle, _target_angle, direction, speed);
        delay(20);
    }
}

/**
 * @brief 保持車子離目標指定的距離，若太近則遠離，若太遠則接近，誤差為+-0.5cm
 * 
 * @param sensor 要感測的sensor
 * @param approach_dir 接近用的移動編碼
 * @param away_dir 遠裡用的移動編碼
 * @param distance 指定的距離，讓車子離目標保持這個距離
 */
void goto_keep_distance(byte sensor, char approach_dir, char away_dir, int distance){

    int now_distance = falldetect.get_distance(sensor);

    while(now_distance != distance){
        now_distance = falldetect.get_distance(sensor);
        if (now_distance > distance){
            // if (now_distance - distance > 10){
            //     move(approach_dir, speed_range[2], 10);
            // }else{
                move(approach_dir, speed_range[0], 10);
            // }
        }else{
            // if (distance - now_distance > 10){
            //     move(away_dir, speed_range[1], 10);
            // }else{
                move(away_dir, speed_range[0], 10);
            // }
        }
    }
    move('p', 0, 10);

}

/**
 * @brief 自動走到邊邊界，輸入要用哪個sensor來當作邊界感測，感測到掉落時即到達
 * 
 * @param sensor sensor的PIN
 * @param direction 要移動的方向
 */
void goto_edge(byte sensor, char direction, int speed){
    while(!falldetect.get_is_fall(sensor)){
        move(direction, speed, 10);
        // commu.send_motor_mega(now_angle, target_angle, direction, speed);
    }
    move('p', 0, 10);
}

/**
 * @brief 自動走到某個東西前
 * 
 * @param sensor 要偵測的sensor
 * @param direction 要往哪邊走
 * @param speed 速度
 * @param distance 要偵測的距離，在小於這個距離錢都會不斷往前走
 */
void goto_until_detect(byte sensor, char direction, int speed, int distance){
    while(falldetect.get_distance(sensor) >= distance){
        move(direction, speed, 10);
        // commu.send_motor_mega(now_angle, target_angle, 'direction', speed);
    }
    move('p', 0, 10);
}


/**
 * @brief 自動走，直到離開某個東西
 * 
 * @param sensor 要偵測的sensor
 * @param direction 要往哪邊走
 * @param speed 速度
 * @param distance 要偵測的距離，在大於這個距離錢都會不斷往前走
 */
void goto_until_no_detect(byte sensor, char direction, int speed, int distance){
    while(falldetect.get_distance(sensor) <= distance){
        move(direction, speed, 40);
    }
    move('p', 0, 10);
}

void toward_target_angle(){
    now_angle = commu.get_angle();
    int _now_angle = now_angle;
    int _target_angle = target_angle;

    if (_target_angle == 180){
        _target_angle = 0;
        if (_now_angle > 0){
            _now_angle = _now_angle - 180;
        }else{
            _now_angle = (_now_angle + 180);
        }
    }
    int angle = _now_angle - _target_angle;
    int angle_unsigned = abs(angle);
    // Serial.println(angle);

    while (angle_unsigned > 0){
        while (angle_unsigned > 0){
            if (angle < -180){
                angle += 360;
                angle_unsigned = 360 - angle_unsigned;
            }

            commu.send_motor_mega(
                _now_angle, _target_angle, 
                angle > 0 ? 'n' : 'm', 
                1.25 * (map(angle_unsigned, 0, 180, 20, 100))
            );
            delay(40);

            now_angle = commu.get_angle();
            _now_angle = now_angle;
            _target_angle = target_angle;
            if (_target_angle == 180){
                _target_angle = 0;
                if (_now_angle > 0){
                    _now_angle = _now_angle - 180;
                }else{
                    _now_angle = (_now_angle + 180);
                }
            }
            angle = _now_angle - _target_angle;
            angle_unsigned = abs(angle);
        }
        commu.send_motor_mega(_now_angle, _target_angle, 'p', 0);
        delay(500);

        now_angle = commu.get_angle();
        _now_angle = now_angle;
        _target_angle = target_angle;
        if (_target_angle == 180){
            _target_angle = 0;
            if (_now_angle > 0){
                _now_angle = _now_angle - 180;
            }else{
                _now_angle = (_now_angle + 180);
            }
        }
        angle = _now_angle - _target_angle;
        angle_unsigned = abs(angle);
        // Serial.println(angle);
    }
}

void toward_target_angle_2(){
    now_angle = commu.get_angle();
    int _now_angle = now_angle;
    int _target_angle = target_angle;

    if (_target_angle == 180){
        _target_angle = 0;
        if (_now_angle > 0){
            _now_angle = _now_angle - 180;
        }else{
            _now_angle = (_now_angle + 180);
        }
    }
    int angle = _now_angle - _target_angle;
    int angle_unsigned = abs(angle);
    // Serial.println(angle);

    // while (angle_unsigned > 0){
    while (angle_unsigned > 0){
        if (angle < -180){
            angle += 360;
            angle_unsigned = 360 - angle_unsigned;
        }

        commu.send_motor_mega(
            _now_angle, _target_angle, 
            angle > 0 ? 'n' : 'm', 
            1 * (map(angle_unsigned, 0, 180, 20, 100))
        );
        delay(40);

        now_angle = commu.get_angle();
        _now_angle = now_angle;
        _target_angle = target_angle;
        if (_target_angle == 180){
            _target_angle = 0;
            if (_now_angle > 0){
                _now_angle = _now_angle - 180;
            }else{
                _now_angle = (_now_angle + 180);
            }
        }
        angle = _now_angle - _target_angle;
        angle_unsigned = abs(angle);
        // }
        // commu.send_motor_mega(_now_angle, _target_angle, 'p', 0);
        // delay(500);

        // now_angle = commu.get_angle();
        // _now_angle = now_angle;
        // _target_angle = target_angle;
        // if (_target_angle == 180){
        //     _target_angle = 0;
        //     if (_now_angle > 0){
        //         _now_angle = _now_angle - 180;
        //     }else{
        //         _now_angle = (_now_angle + 180);
        //     }
        // }
        // angle = _now_angle - _target_angle;
        // angle_unsigned = abs(angle);
        // Serial.println(angle);
    }
}