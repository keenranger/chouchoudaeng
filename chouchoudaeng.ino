int dryer_pin[3] = {48, 49, 50};
int button_pin[9] = {22, 24, 26, 28, 30, 32, 34, 36, 38}; //물받기.샴푸.월풀.스파.헹굼.욕조청소.일시정지.드라이1.드라이2
int led_pin[9] = {23, 25, 27, 29, 31, 33, 35, 37, 39}; //같은순서
int relay_pin[8] = {40, 41, 42, 43, 44, 45, 46, 47};
int human_pin = 51;
int pay_pin = 52;
int buzzer_pin = 2;
int water_pin = 15;
///////these lines are for debouncing buttons///////////
boolean last_button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int last_debounce[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////

boolean button_toggle[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int state = 0;//0:정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소
int pump_state = 0; //0:꺼짐, 1:켜짐대기 or 켜짐
int pump_prev = 0; //펌프 딜레이를 위한 부분
int pay_prev = 0; //방금전에 결제가 되어있던 상태인지 확인하기 위한 부분
int auto_clean_queue = 0; //자동세척 해야함
int auto_clean_prev = 0;



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

void setup() {
  for (int i = 0; i < 9; i++) { //Set button pins as input
    pinMode(button_pin[i], INPUT);
  }
  for (int i = 0; i < 9; i++) { //Set led pins as output
    pinMode(led_pin[i], OUTPUT);
  }
  for (int i = 0; i < 8; i++) { //Set relay pins as output
    pinMode(relay_pin[i], OUTPUT);
  }
  for (int i = 0; i < 3; i++) { //Set dryer pins as output
    pinMode(dryer_pin[i], OUTPUT);
  }
  pinMode(human_pin, INPUT);
  pinMode(pay_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  //pinMode(water_pin, INPUT); 아날로그 read는 핀모드 안해도댐니다
}

void loop() {
  if (digitalRead(pay_pin) == HIGH) {//결제 되었을 때 만
    for (int i = 0; i < 9; i++) { //버튼 받아오기
      debouncing_button(i);
    }
    pay_prev = 1;
  }
  else {//결제가 끝났을때의 경우
    if (pay_prev == 1) {//결제가 켜져있다가 꺼진 경우이면
      state_0();//우선 정지상태로
      button_toggle[7]=0;//드라이어 버튼 상태도 꺼줍니다.
      button_toggle[8]=0;
      pay_prev = 0;
      auto_clean_queue = 1; //자동세척 한번 예약
    }
    auto_clean_check();
  }
  pump_check();
  led_check();
  dryer_check;
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
    button_toggle[0] = 1;
    pump_start(); //펌프키고
    digitalWrite(relay_pin[1], HIGH); //급수키고
    button_check; //샴월스끄고
    digitalWrite(relay_pin[5], LOW); //배관세척끄고
    digitalWrite(relay_pin[6], LOW); //욕조세척끄고
    digitalWrite(relay_pin[7], HIGH); //배수키고
  }
  else {//물받기 끌때
    state_0(); //정지상태로
  }
}
void state_2(int i) {
  if (state != 2) {//샴.월.스 처음 킬때
    if (analogRead(water_pin) >= 500) {
      toggle_reset();
      state = 2;
      button_toggle[i] = 1;
      pump_start(); //펌프키고
      digitalWrite(relay_pin[0], LOW); //급수끄고
      button_check; //샴월스 키고꺼기 체크
      digitalWrite(relay_pin[5], LOW); //배관세척끄고
      digitalWrite(relay_pin[6], LOW); //욕조세척끄고
      digitalWrite(relay_pin[7], HIGH); //배수키고
    }
    else {
      //todo 수위 확인하라는 메시지
    }
  }
  else {//샴.월.스 토글
    button_toggle[i] = !button_toggle[i];
    button_check();//버튼 토글상태에따른 핀 조절 ->샴월스 담당
  }
}
void state_3() {
  toggle_reset();
  if (state != 3) {//헹굼 킬때
    state = 3;
    button_toggle[4] = 1;
    pump_start(); //펌프키고
    digitalWrite(relay_pin[1], HIGH); //급수키고
    button_check; //샴월스 키고꺼기 체크
    digitalWrite(relay_pin[5], LOW); //배관세척끄고
    digitalWrite(relay_pin[6], LOW); //욕조세척끄고
    digitalWrite(relay_pin[7], LOW); //배수끄고
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
  //todo 음성으로 시작하는것 알려주기
  digitalWrite(relay_pin[1], HIGH); //급수키고
  button_check; //샴월스 키고꺼기 체크
  digitalWrite(relay_pin[5], LOW); //배관세척끄고
  digitalWrite(relay_pin[6], HIGH); //욕조세척키고
  digitalWrite(relay_pin[7], LOW); //배수끄고
  delay(3000);//3초후에
  digitalWrite(relay_pin[0], HIGH);//3초후 펌프키고
  delay(15000);//15초 작동 후
  digitalWrite(relay_pin[0], LOW);//펌프끄고
  digitalWrite(relay_pin[6], LOW); //욕조세척끄고
  state_0();
}
void state_0() {//정지버튼이나 토글로 꺼질 때
  toggle_reset();
  state = 0;
  button_toggle[6] = 1;
  pump_stop(); //펌프끄고
  digitalWrite(relay_pin[1], LOW);
  button_check(); //샴월스끄고
  digitalWrite(relay_pin[5], LOW); //배관끄고
  digitalWrite(relay_pin[6], LOW); //욕조끄고
}
///자동세척 부분///
void auto_clean_check() {
  if (auto_clean_queue == 1 && digitalRead(human_pin) == LOW) {//대기중이고, 사람없으면
    if (auto_clean_prev > 0) {//카운트초기화되어있으면
      auto_clean_prev = millis(); //카운트할당
    }
    if ((millis() - auto_clean_prev) > 60000) {//60초되면
      //todo 음성으로 시작하는것 알려주기
      digitalWrite(relay_pin[1], HIGH); //급수키고
      button_check; //샴월스 키고꺼기 체크
      digitalWrite(relay_pin[5], HIGH); //배관세척키고
      digitalWrite(relay_pin[6], LOW); //욕조세척끄고
      digitalWrite(relay_pin[7], LOW); //배수끄고
      delay(3000);//3초후에
      digitalWrite(relay_pin[0], HIGH);//3초후 펌프키고
      delay(15000);//15초후에
      digitalWrite(relay_pin[5], LOW); //배관세척끄고
      digitalWrite(relay_pin[6], HIGH); //욕조세척키고
      delay(15000);//15초후에
      digitalWrite(relay_pin[0], LOW);//펌프끄고
      digitalWrite(relay_pin[6], LOW); //욕조세척끄고
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
  pump_prev = millis();
}
void pump_stop() {
  pump_state = 0;
}
void pump_check() {
  if (pump_state == 1 && (millis() - pump_prev) > 3000) {//3초후 켜지게
    digitalWrite(relay_pin[0], HIGH);
  }
  else {
    digitalWrite(relay_pin[0], LOW);
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
    if (button_toggle[i + 2] == 1) {
      digitalWrite(relay_pin[i + 2], HIGH);
    }
    else {
      digitalWrite(relay_pin[i + 2], LOW);
    }
  }
}
void dryer_check() { //토글에따른 릴레이 제어 부분
  if (button_toggle[7] == 1 || button_toggle[8] == 1) {
    digitalWrite(dryer_pin[0], HIGH);
  }
  if (digitalRead(pay_pin) == LOW) {
    digitalWrite(dryer_pin[0], LOW);
  }
  for (int i = 0; i < 2; i++) { //dryer onoff upon toggle
    if (button_toggle[i + 7] == 1) {
      digitalWrite(dryer_pin[i + 1], HIGH);
    }
    else {
      digitalWrite(dryer_pin[i + 1], LOW);
    }
  }
}
void buzzer() {
  tone(buzzer_pin, 880, 20);
  delay(20);
}

void toggle_reset() { //bath쪽 toggle 리셋
  for (int i = 0; i < 7; i++) {
    button_toggle[i] = 0;
  }
}
