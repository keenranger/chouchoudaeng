#include <Wire.h>
#define SLAVE 4

void pinmode_input();
void pinmode_output();

// 핀번호들 모음
int water_pin[2] = {68, 69};  // [A14, A15] = [68, 67] = [샴푸물수위 넘침/충분]
int dryer_pin[3] = {48, 49, 50};  //집진기,드라이1,드라이2
int relay_pin[8] =
    {40, 41, 42, 43,
     44, 45, 46, 47};  //펌프,스파, 밸브,급수 밸브,샤워 밸브,월풀
                       //밸브,배관 세척 밸브,욕조 청소 밸브,배수 밸브
int shampoo_pin = 51;  //샴푸투입
int human_pin = 53;    //인체감지센서

void setup() {
    Serial.begin(115200);
    Wire.begin(SLAVE);  //위에서 define한 주소로 slave 등록
    Wire.onReceive(receiveFromMaster);
    Wire.onRequest(sendToMaster);
    // pinmode
    pinmode_input();
    pinmode_output();
}

void loop() {
    Serial.print(analogRead(water_pin[0]));
    Serial.print("\t");
    Serial.println(analogRead(water_pin[1]));
    delay(100);
}
void receiveFromMaster(int bytes) {}

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

void sendToMaster() { //마스터가 데이터 요청하면 전송해줌
    int data[2] = {0, 0};
    for (int i = 0; i < 2; i++) {
        int water_level = analogRead(water_pin[i]);
        if (water_level > 500) {
            data[i] = 1;  // [A14, A15] = [68, 67] = [샴푸물수위 넘침/충분]
        }
    }
    Wire.write(data[0] * 10 + data[1]);
}
