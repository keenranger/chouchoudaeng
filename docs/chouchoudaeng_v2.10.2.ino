#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>


int dryer_pin[3] = {48, 49, 50};
int button_pin[9] = {22, 24, 28, 32, 26, 30, 34, 36, 38}; //물받기.샴푸.헹굼.월풀.욕조청소.스파.일시정지.드라이1.드라이2
int led_pin[9] = {23, 25, 29, 33, 27, 31, 35, 37, 39}; //같은순서
int relay_pin[8] = {40, 41, 42, 43, 44, 45, 46, 47}; //펌프,스6파 밸브,급수 밸브,샤워 밸브,월풀 밸브,배관 세척 밸브,욕조 청소 밸브,배수 밸브
int human_pin = 51;
int pay_pin = 52;
int buzzer_pin = 2;
int shampoo_add_button = 12;
int auto_shampoo_relay = 53;
int water_pin = A15; // 샴푸 물 수위 확인
int water_limit_pin = A14 ;  // 수위 넘침 조절 핀

///////these lines are for debouncing buttons///////////
boolean last_button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long last_debounce[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////

boolean button_toggle[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int state = 0;//0:정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소
int pump_state = 0; //0:꺼짐, 1:켜짐대기 or 켜짐
unsigned long pump_prev = 0; //펌프 딜레이를 위한 부분
int pay_prev = 0; //방금전에 결제가 되어있던 상태인지 확인하기 위한 부분
int auto_clean_queue = 0; //자동세척 해야함
unsigned long shower_time_prev = 0;
unsigned long rinse_time_prev = 0;
unsigned long auto_clean_prev = 0;

unsigned long water_limit_1_prev = 0;
int water_limit_1_state = 0;

int shampoo_cnt = 0;
int water_state_1 = 0;  // 물받기 확인
int rinse_state = 0;  // 헹굼 확인
int wellcome_state = 0;
int water_lmint_stat = 0;// 물 넘침 확인
int dry_state_1 = 0;
int dry_state_2 = 0;
int shampoo_state = 0;
int wallpool_state = 0;
int spa_state = 0;
int auto_shampoo_state = 0;
unsigned long auto_shampoo_prev = 0;
int shampoo_state_2 = 0;
int shampoo_button_prev = 1;
int shampoo_add_state = 0;


void debouncing_button(int i);
void state_0();
void state_1();
void state_2(int i);
void state_3();
void state_4();
void auto_clean_check();
void pump_start();
void pump_stop();
void pump_check();
void button_check();
void led_check();
void dryer_check();
void buzzer();
void toggle_reset();
void auto_shampoo_start();
void auto_shampoo_check();
void auto_shampoo_stop();
DFPlayerMini_Fast myMP3;


void setup() {
  Serial.begin(9600);
  Serial2.begin(115200);
  Serial3.begin(9600);

  myMP3.begin(Serial3);
  myMP3.volume(30);
  delay(20);

  for (int i = 0; i < 9; i++) { //Set button pins as input
    pinMode(button_pin[i], INPUT);
  }

  for (int i = 0; i < 9; i++) { //Set led pins as output
    pinMode(led_pin[i], OUTPUT);
  }

  for (int i = 0; i < 8; i++) { //Set relay pins as output
    pinMode(relay_pin[i], OUTPUT);
    digitalWrite(relay_pin[i], HIGH);
  }

  for (int i = 0; i < 3; i++) { //Set dryer pins as output
    pinMode(dryer_pin[i], OUTPUT);
    digitalWrite(dryer_pin[i], HIGH);
  }

  pinMode(human_pin, INPUT);
  pinMode(pay_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(auto_shampoo_relay , OUTPUT);
  digitalWrite(auto_shampoo_relay , HIGH);

}


void loop() {
  Serial.println((millis() - auto_clean_prev));

  if (Serial2.available()) {
    String cmd = Serial2.readStringUntil('\n');
    int cmd_len = cmd.length();
    Serial.println(cmd);
    if ( cmd == "3M" ) {
      myMP3.play (15);    //0015_3분남음
    }

    if (cmd == "1M") {
      myMP3.play (16);    //0016_1분남음
    }

    if (cmd == "clean") {
      myMP3.play (12);    //0015_자동세척
      state_4();
    }

  }

  //Serial.println(analogRead(water_pin));
  //Serial.println(analogRead(water_limit_pin));


  if (digitalRead(pay_pin) == HIGH) {//결제 되었을 때 만0


    if (  digitalRead(shampoo_add_button) == LOW && shampoo_add_state < 3 ) {  //LED버튼 누르면 샴푸투입 최대 3회
      {
        Serial.print("샴푸투입");
        Serial.println( shampoo_add_state);
        shampoo_add_state = shampoo_add_state + 1;
        digitalWrite(auto_shampoo_relay, LOW);
        myMP3.play (5); // 샴푸투입 버튼 
        delay(5000);
        digitalWrite(auto_shampoo_relay, HIGH);
        Serial.println("샴푸투입 종료");
      }
    }


    for (int i = 0; i < 9; i++) { //버튼 받아오기
      debouncing_button(i);

      if ((millis() - shower_time_prev)  > 10000 && button_toggle[2] == 0 && state == 2 ) {  //샴푸 누르면 월풀 10초후에 꺼짐
        digitalWrite(relay_pin[3], HIGH); //월풀끄고
        shampoo_cnt = 0;
      }

      if ((millis() - rinse_time_prev ) > 10000 &&  state == 3) {   //헹굼 누르면 월풀 10초후에 꺼짐
        digitalWrite(relay_pin[3], HIGH); //월풀끄고
      }
    }

    if (pay_prev == 0) {
      myMP3.play (1);    //0001_결제완료
    }

    pay_prev = 1;

  }

  else {//결제가 끝났을때의 경우

    if (pay_prev == 1) {//결제가 켜져있다가 꺼진 경우이면

      pay_prev = 0;//결제가 끝났다고 알려줌.

      state_0();//우선 정지상태로

      button_toggle[7] = 0; //드라이어 버튼 상태도 꺼줍니다.

      button_toggle[8] = 0;

      auto_clean_queue = 1; //자동세척 한번 예약

      water_state_1 = 0;  // 물받기 초기화

      rinse_state = 0;   // 헹굼 초기화

      wellcome_state = 0;  // 웰컴 초기화

      // auto_shampoo_stop();

      auto_shampoo_state = 0;

      dry_state_1 = 0;

      dry_state_2 = 0;

      shampoo_state_2 = 0;

      shampoo_add_state = 0; //샴푸 투입 횟수 초기화

      water_limit_1_state = 0; //수위확인 초기화

      myMP3.play (17);    //0017_시간종료
      auto_clean_prev = millis();

      //todo 시간 종료

    }
    auto_clean_check();

  }


  led_check();

  pump_check();

  dryer_check();

  //auto_shampoo_check();



  water_limit_1_check();  //20초 지나고 수위 확인
}

///////////// 버튼 인식 부분 ////////////////

void debouncing_button(int i) {

  int reading = digitalRead(button_pin[i]);

  if (reading != last_button_state[i])  //스위치의 이전과 지금 상태가 다르면

    last_debounce[i] = millis();   //초를 기록합니다.



  if ((millis() - last_debounce[i]) > debounce_delay) { //스위치 상태가 debounce_delay 이상 같으면

    if (reading != button_state[i]) { //state와 상태가 다를경우 기록

      button_state[i] = reading;

      if (button_state[i] == LOW) { //LOW일때 작업 할당 -> pull-up 저항있음

        buzzer();//버튼누를때마다 소리나게

        switch (i) {//누른 버튼에 따라 적당한 state로 할당

          case 0:

            state_1();

            break;

          case 1 ... 3:

            state_2(i);

            break;

          case 4:

            state_3();

            break;

          case 5:

            state_4();

            break;

          case 6:

            state_0();

            break;

          default:

            button_toggle[i] = !button_toggle[i];


            break;

        }

      }

    }

  }

  last_button_state[i] = reading;

}

/////////// state 부분 /////////

void state_1() {

  toggle_reset();

  if (state != 1) {// 물받기 킬때
    state = 1;
    myMP3.play (2);    //0002_물받기

    button_toggle[0] = 1;
    pump_start(); //펌프키고
    digitalWrite(relay_pin[1], LOW); //급수키고
    button_check(); //샴월스끄고
    digitalWrite(relay_pin[3], LOW); //월풀키고
    digitalWrite(relay_pin[5], LOW); //배관세척키고
    digitalWrite(relay_pin[6], HIGH); //욕조세척끄고
    digitalWrite(relay_pin[7], LOW); //배수키고
    water_limit_1_start(); //  작은욕조 수위 확인 시작

  }

  else {//물받기 끌때

    state_0(); //정지상태로

  }

}

void state_2(int i) {

  if (state != 2) {//샴.월.스 처음 킬때

    if (analogRead(water_pin) < 500) {

      toggle_reset();

      state = 2;

      button_toggle[i] = 1;

      pump_start(); //펌프키고

      if ((button_toggle[1] == 0) && (button_toggle[2] == 0)) { //샴월둘다꺼지면

        pump_stop();

      }

      digitalWrite(relay_pin[1], HIGH); //급수끄고

      button_check(); //샴월스 키고꺼기 체크

      digitalWrite(relay_pin[3], LOW); //월풀키고

      digitalWrite(relay_pin[5], HIGH); //배관세척끄고

      digitalWrite(relay_pin[6], HIGH); //욕조세척끄고

      digitalWrite(relay_pin[7], LOW); //배수키고

    }

    else {
      myMP3.play (4);    //0004_수위확인
      //todo 수위 확인하라는 메시지

    }

  }

  else {//샴.월.스 토글

    button_toggle[i] = !button_toggle[i]; //해당버튼 토글하고

    digitalWrite(relay_pin[3], LOW); //월풀키고

    if ((button_toggle[1] == 0) && (button_toggle[2] == 0)) { //샴월둘다꺼지면

      pump_stop();

    }

    else { //하나라도 켜지면

      pump_start();

    }

    button_check();//버튼 토글상태에따른 핀 조절 ->샴월스 담당

  }

}

void state_3() {

  toggle_reset();

  if (state != 3) {//헹굼 킬때

    state = 3;

    myMP3.play (9);    //0009_헹굼시작
    //todo 잠시 후 헹굼이 시작됩니다.샤워기를 꼭 잡아주세요.


    button_toggle[4] = 1;

    rinse_time_prev = millis();

    pump_start(); //펌프키고

    digitalWrite(relay_pin[1], LOW); //급수키고

    button_check(); //샴월스 키고꺼기 체크

    digitalWrite(relay_pin[2], LOW); //샤워기키고

    digitalWrite(relay_pin[3], LOW); //월풀키고



    digitalWrite(relay_pin[5], HIGH); //배관세척끄고

    digitalWrite(relay_pin[6], HIGH); //욕조세척끄고

    digitalWrite(relay_pin[7], HIGH); //배수끄고

  }

  else {//헹굼 끌때

    state_0();

  }

}

void state_4() {//욕조청소는 다른 버튼이 눌리지 않는다.

  toggle_reset();

  state = 4;

  button_toggle[5] = 1;

  led_check(); //루프로 안가기때문에

  myMP3.play (10);    //0010_자동욕조배관청소
  //todo 음성으로 시작하는것 알려주기

  digitalWrite(relay_pin[1], LOW); //급수키고

  button_check(); //샴월스 키고꺼기 체크

  digitalWrite(relay_pin[5], HIGH); //배관세척끄고

  digitalWrite(relay_pin[6], LOW); //욕조세척키고

  digitalWrite(relay_pin[7], HIGH); //배수끄고

  delay(3000);//3초후에

  digitalWrite(relay_pin[0], LOW);//3초후 펌프키고

  delay(25000);//15초 작동 후

  digitalWrite(relay_pin[6], HIGH); //욕조세척끄고

  digitalWrite(relay_pin[5], LOW); //배관세척키고

  delay(10000);//10초 작동 후

  digitalWrite(relay_pin[5], HIGH); //배관세척끄고

  digitalWrite(relay_pin[0], HIGH);//펌프끄고


  state_0();

}

void state_0() {//정지버튼이나 토글로 꺼질 때


  toggle_reset();

  state = 0;

  button_toggle[6] = 1;

  pump_stop(); //펌프끄고


  digitalWrite(relay_pin[1], HIGH);

  button_check(); //샴월스끄고

  digitalWrite(relay_pin[5], HIGH); //배관끄고

  digitalWrite(relay_pin[6], HIGH); //욕조끄고

  if ( pay_prev == 1) { //결제 중에만 기능 정지 멘트

    myMP3.play (11);    //0011_기능정지

  }

  if (pay_prev == 0) { //결제종료로 오는 경우라면

    digitalWrite(relay_pin[7], HIGH); //배수끄고(물이빠짐)

  }


}
///자동세척 부분///

void auto_clean_check() {

  if (auto_clean_queue == 1 ){//대기중이고, 사람없으면

    if (auto_clean_prev > 0 && (millis() - auto_clean_prev) > 600000) {//600초되면

      myMP3.play (12);    //0012_자동욕조배관세척시작

      //todo 음성으로 시작하는것 알려주기

      digitalWrite(relay_pin[1], LOW); //급수키고

      button_check(); //샴월스 키고꺼기 체크

      digitalWrite(relay_pin[5], LOW); //배관세척키고

      digitalWrite(relay_pin[6], HIGH); //욕조세척끄고

      digitalWrite(relay_pin[7], HIGH); //배수끄고

      delay(3000);//3초후에

      digitalWrite(relay_pin[0], LOW);//3초후 펌프키고

      delay(15000);//25초후에

      digitalWrite(relay_pin[5], HIGH); //배관세척끄고

      digitalWrite(relay_pin[6], LOW); //욕조세척키고

      delay(25000);//15초후에

      digitalWrite(relay_pin[0], HIGH);//펌프끄고

      digitalWrite(relay_pin[6], HIGH); //욕조세척끄고

      auto_clean_queue = 0; //작업완료 확인하고

      state_0();

    }

  }

  else {//사람오면

    auto_clean_prev = 0;//카운트초기화

  }

}

//////////////  펌프부분  ////////////////////

void pump_start() {

  pump_state = 1;

  if (pump_prev == 0) {


    pump_prev = millis();

  }

}

void pump_stop() {

  pump_state = 0;

  pump_prev = 0;

}

void pump_check() {

  if (pump_state == 1 && (millis() - pump_prev) > 5000) {//5초후 켜지게

    digitalWrite(relay_pin[0], LOW);

  }

  else {

    digitalWrite(relay_pin[0], HIGH);

  }

}

//////////////////////////////

void led_check() {

  for (int i = 0; i < 9; i++) { //led on/off upon toggle

    if (button_toggle[i] == 1) {

      digitalWrite(led_pin[i], HIGH);


    }

    else {

      digitalWrite(led_pin[i], LOW);

    }

  }

}

void button_check() { //토글에따른 릴레이 제어 부분 ->샴월스
  for (int i = 0; i < 3; i++) { //led on/off upon toggle

    if (button_toggle[i + 1] == 1) {

      if ( button_toggle[ 1] == 1 && shampoo_state == 0) {
        myMP3.play (6);  //0006_샴푸
        shampoo_state = 1;
      }
      else if ( button_toggle[ 2] == 1 && wallpool_state == 0) {
        myMP3.play (7);  //0007_월풀
        wallpool_state = 1;
      }
      else if ( button_toggle[ 3] == 1 && spa_state == 0) {
        myMP3.play (8);  //0008_스파
        spa_state = 1;
      }

      digitalWrite(relay_pin[i + 2], LOW);


    }

    else {

      digitalWrite(relay_pin[i + 2], HIGH);

      if ( button_toggle[ 1] == 0 && shampoo_state == 1) {
        shampoo_state = 0;
      }
      else if ( button_toggle[ 2] == 0 && wallpool_state == 1) {
        wallpool_state = 0;
      }
      else if ( button_toggle[ 3] == 0 && spa_state == 1) {
        spa_state = 0;
      }


      if (button_toggle[1] == 1 && shampoo_cnt == 0) {

        shower_time_prev = millis();

        digitalWrite(relay_pin[3], LOW); //월풀키고

        shampoo_cnt = 1;

      }

    }

  }


}

void dryer_check() { //토글에따른 릴레이 제어 부분

  if (button_toggle[7] == 1 || button_toggle[8] == 1) {//집진기 둘중하나라도 켜지면 켜기


    digitalWrite(dryer_pin[0], LOW);

  }

  if (digitalRead(pay_pin) == LOW) {//결제끝나면 끄기

    digitalWrite(dryer_pin[0], HIGH);

  }

  for (int i = 0; i < 2; i++) { //dryer onoff upon toggle

    if (button_toggle[i + 7] == 1) {

      if ( button_toggle[ 7] == 1 && dry_state_1 == 0) {
        myMP3.play (13);  //0013_드라이1
        dry_state_1 = 1;
      }

      if (button_toggle[ 8] == 1 && dry_state_2 == 0) {
        myMP3.play (14);  //0014_드라이2
        dry_state_2 = 1;
      }
      digitalWrite(dryer_pin[i + 1], LOW);

    }

    else {
      if ( button_toggle[ 7] == 0 && dry_state_1 == 1) {
        dry_state_1 = 0;

      }
      if ( button_toggle[ 8] == 0 && dry_state_2 == 1) {
        dry_state_2 = 0;
      }
      digitalWrite(dryer_pin[i + 1], HIGH);

    }

  }

}

void buzzer() {

  tone(buzzer_pin, 880, 50);

  delay(30);

}



void toggle_reset() { //bath쪽 toggle 리셋

  for (int i = 0; i < 7; i++) {

    button_toggle[i] = 0;

  }

}


void water_limit_1_start() {
  if (water_limit_1_state == 0) {  //처음 한번만 
    water_limit_1_prev = millis();
    water_limit_1_state = 1;
  }
}


void water_limit_1_check() {
  if (water_limit_1_state == 1 && (millis() - water_limit_1_prev) > 50000)
    if (analogRead(water_pin) < 300) {
      pump_stop();
      digitalWrite(relay_pin[1], HIGH); //급수끄고
      myMP3.play (3);  //0003 수위 확인 
      water_limit_1_state = 2;
      Serial.println("작은 욕조 수위 확인");
    }

}
