#include <Wire.h>
#define SLAVE 4

void pinmode_input();
void pinmode_output();

// 핀번호들 모음
int water_pin[2] = {68, 69};      // A14, A15 = 68, 67
int dryer_pin[3] = {48, 49, 50};  //집진기,드라이1,드라이2
int relay_pin[8] =
    {40, 41, 42, 43,
     44, 45, 46, 47};  //펌프,스파, 밸브,급수 밸브,샤워 밸브,월풀
                       //밸브,배관 세척 밸브,욕조 청소 밸브,배수 밸브
int shampoo_pin = 51;  //샴푸투입
int human_pin = 53;    //인체감지센서

void setup() {
    Wire.begin(SLAVE);  //위에서 define한 주소로 slave 등록
    Wire.onReceive(receiveFromMaster);
    Wire.onRequest(sendToMaster);
    // pinmode
    pinmode_input();
    pinmode_output();
}

void loop() {}
void receiveFromMaster(int bytes) {}

void sendToMaster() {
    for (int i = 0; i < 2; i++) {
        Wire.write(analogRead(water_pin[i]));
    }
}
//핀모드용 함수들
void pinmode_input() { pinMode(human_pin, INPUT); }
void pinmode_output() {
    for (int i = 0; i < 3; i++) {
        pinMode(dryer_pin[i], OUTPUT);
    }
    for (int i = 0; i < 8; i++) {
        pinMode(relay_pin[i], OUTPUT);
    }
    pinMode(shampoo_pin, OUTPUT);
}