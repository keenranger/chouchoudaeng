#include <Wire.h>
////////// 핀번호들 //////////
int buzzer_pin = 2;
int led_pin[9] = {23, 25, 27, 29, 31, 33, 35, 37, 39};  //같은순서
int pay_pin = 52;

///////these lines are for debouncing buttons///////////
boolean last_button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long last_debounce[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////
boolean button_toggle[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int state = 0;  // 0:정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소

void setup() {}
void loop() {}

//핀모드용 함수들
void pinmode_input() { pinMode(pay_pin, INPUT); }
void pinmode_output() {
    for (int i = 0; i < 9; i++) {
        pinMode(led_pin[i], OUTPUT);
    }
    pinMode(buzzer_pin, OUTPUT);
}