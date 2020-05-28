#include <DFPlayerMini_Fast.h>
#include <Wire.h>
////////// 핀번호들 //////////
int buzzer_pin = 2;
int led_pin[9] = {23, 25, 27, 29, 31, 33, 35, 37, 39};  //같은순서
int pay_pin = 52;

///////these lines are for debouncing buttons///////////
int button_pin[10] = {
    22, 24, 28, 32, 26, 30,
    34, 36, 38, 12};  //물받기.샴푸.헹굼.월풀.욕조청소.스파.일시정지.드라이1.드라이2,
                      //샴푸추가버튼
boolean last_button_state[10] = {false, false, false, false, false,
                                 false, false, false, false, false};
boolean button_state[10] = {false, false, false, false, false,
                            false, false, false, false, false};
boolean button_toggle[10] = {false, false, false, false, false,
                             false, false, false, false, false};
boolean button_queue[10] = {false, false, false, false, false,
                            false, false, false, false, false};
unsigned long last_debounce[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 100;  // 100ms로 debounce_delay설정

/////////////////////////////////////////////////////////
int state = 0;  // 0:정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소

DFPlayerMini_Fast voice_guidance;  //음성안내를 위한 객체

void setup() {
    Serial2.begin(115200);  // nodemcu 통신에 사용
    Serial3.begin(9600);    //스피커 부분에 사용

    voice_guidance.begin(Serial3);  //스피커 초기화 하는 부분
    voice_guidance.volume(30);      //음성안내 볼륨을 30으로
    delay(20);

    pinmode_input();   //센서, 버튼 등 input들 일괄 처리
    pinmode_output();  // led 등 output들 일괄 처리
}
void loop() {}

//핀모드용 함수들
void pinmode_input() { pinMode(pay_pin, INPUT); }
void pinmode_output() {
    for (int i = 0; i < 9; i++) {  //
        pinMode(led_pin[i], OUTPUT);
    }
    pinMode(buzzer_pin, OUTPUT);
}

void button_check(int i) { //버튼 안정적으로 읽기 위한 부분
    boolean reading = digitalRead(button_pin[i]);
    if (reading != last_button_state)  //스위치의 electrical state가 변화했다면
        last_debounce[i] = millis();
    if ((millis() - last_debounce[i]) >
        debounce_delay) {  // electrical state가 debounce_delay 이상 같으면
        if (reading != button_state[i]) {  //눌르고있어도 한번만 처리하기위해
            button_state[i] = reading;  // button_state 현재 electrical state로
            if (button_state[i] == false)  // pullup 이기에, low일떄 누른 것
                button_queue[i] = true;  //처리를 위해 queue에 추가
        }
    }
    last_button_state = reading;
}
