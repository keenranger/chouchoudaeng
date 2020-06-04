#include <DFPlayerMini_Fast.h>
#include <Wire.h>
#define SLAVE 4
////////// 핀번호들 //////////
int buzzer_pin = 2;  //버튼 비프음 내기 위함
int led_pin[9] = {23, 25, 27, 29, 31, 33, 35, 37, 39};  //같은순서
int human_pin = 51;                                     //인체감지센서
int pay_pin = 52;                                       // 결제되면 high
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
unsigned long last_request = 0;
const int debounce_delay = 100;  // 100ms로 debounce_delay설정
/////////////////////////////////////////////////////////
boolean pay_prev = false;  //방금전에 결제상태 였는가?
boolean water_level[2] = {false, false};
int state = 0;  // 0:정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소
int shampoo_chance = 3;            //샴푸 추가 3회 사용 가능
DFPlayerMini_Fast voice_guidance;  //음성안내를 위한 객체
void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);  // nodemcu 통신에 사용
    Serial3.begin(9600);    //스피커 부분에 사용

    voice_guidance.begin(Serial3);  //스피커 초기화 하는 부분
    voice_guidance.volume(30);      //음성안내 볼륨을 30으로
    delay(200);

    pinmode_input();   //센서, 버튼 등 input들 일괄 처리
    pinmode_output();  // led 등 output들 일괄 처리
}
void loop() {
    if (pay_check()) {                  //결제 되었을 때만
        for (int i = 0; i < 10; i++) {  //버튼이 10개
            button_check(i);            //버튼 눌렸는지 확인
            queue_processor(i);         //버튼에 따라 동작
        }
    }
    led_check();     // 토글 상태에 따라 LED On/Off
    from_nodemcu();  // nodemcu 명령왔나 확인
    if ((millis() - last_request) > 1000) {  // 1초마다
        last_request = millis();
        request_slave();  // slave에게 수위센서 데이터 요청
    }
}

//////핀모드용 함수들//////////
void pinmode_input() {
    pinMode(human_pin, INPUT);
    pinMode(pay_pin, INPUT);        //결제핀 input 설정
    for (int i = 0; i < 10; i++) {  //버튼들 input 설정
        pinMode(button_pin[i], INPUT);
    }
}
void pinmode_output() {
    for (int i = 0; i < 9; i++) {  //샴푸추가를 뺀 버튼을 위한 led
        pinMode(led_pin[i], OUTPUT);
    }
    pinMode(buzzer_pin, OUTPUT);  //버튼 누르는 소리
}
//////////////////////////////
void button_check(int i) {  //버튼 안정적으로 읽기 위한 부분
    boolean reading = digitalRead(button_pin[i]);
    if (reading != last_button_state)  //스위치의 electrical state가 변화했다면
        last_debounce[i] = millis();
    if ((millis() - last_debounce[i]) >
        debounce_delay) {  // electrical state가 debounce_delay 이상 같으면
        if (reading != button_state[i]) {  //눌르고있어도 한번만 처리하기위해
            button_state[i] = reading;  // button_state 현재 electrical state로
            if (button_state[i] == false) {  // pullup 이기에, low일때 누른 것
                button_queue[i] = true;     //처리를 위해 queue에 추가
                tone(buzzer_pin, 880, 50);  //누를때 마다 버저 소리내기
            }
        }
    }
    last_button_state[i] = reading;
}

void queue_processor(int i) {       //버튼 누른거에 따라 동작 할당
    if (button_queue[i] == true) {  //버튼을 눌러 queue에 들어왔다면
        switch (i) {                //버튼에 따라 알맞은 동작
            case 0:                 //물받기 눌렸다면
                break;
            case 1:  //샴푸 눌렸다면
                break;
            case 2:  //헹굼 눌렸다면
                break;
            case 3:  //월풀 눌렸다면
                break;
            case 4:  //욕조청소 눌렸다면
                break;
            case 5:  //스파 눌렸다면
                break;
            case 6:  //일시정지 눌렸다면
                break;
            case 7:  //드라이1 눌렸다면
                break;
            case 8:  //드라이2 눌렸다면
                break;
            case 9:  //샴푸추가 눌렸다면
                break;
            default:
                button_toggle[i] = !button_toggle[i];
                break;
        }
    }
}

void from_nodemcu() {  // nodemcu로부터 명령 받는 부분
    if (Serial2.available()) {
        String command = Serial2.readStringUntil('\n');
        if (command == "3M") {
            voice_guidance.play(15);  // 0015_3분 남음
        } else if (command == "1M") {
            voice_guidance.play(16);  // 0016_1분 남음
        } else if (command == "clean") {
            voice_guidance.play(12);  // 0012_자동 세척
            // TODO : 자동세척 명령
        }
    }
}
void request_slave() {
    Wire.requestFrom(SLAVE, 2);
    int data = Wire.read();
    if (data >= 10) {
        water_level[0] = true;
    } else {
        water_level[0] = false;
    }
    if (data % 10 == 1) {
        water_level[1] = true;
    } else {
        water_level[1] = false;
    }
}
void to_slave() {  // slave에게 명령 주는 부분
}
boolean pay_check() {                    //돈냈는지 확인
    if (digitalRead(pay_pin) == true) {  //결제 되어있는 상태라면
        if (pay_prev == false) {         //미결제->결제 상황이라면
            voice_guidance.play(1);      // 0001_결제완료
        }
        pay_prev = true;
        return true;
    } else {
        if (pay_prev == true) {  //결제->미결제 상황이라면
            shampoo_chance = 3;  //다음 이용자를 위해 샴푸 3회로 초기화시켜줌
        }
        pay_prev = false;
        return false;
    }
}

void led_check() {
    for (int i = 0; i < 9; i++) {  // led on/off upon toggle
        if (button_toggle[i] == 1) {
            digitalWrite(led_pin[i], HIGH);
        } else {
            digitalWrite(led_pin[i], LOW);
        }
    }
}