boolean dryer_power[2] = {0, 0};
int button_array[9] = {22, 23, 24, 25, 26, 27, 28, 29, 30}; //물받기.샴푸.월풀.스파.헹굼.욕조청소.일시정지.드라이1.드라이2
int led_array[9] = {40, 41, 42, 43, 44, 45, 46, 47, 48}; //46더미핀

///////these lines are for debouncing buttons///////////
boolean last_button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int last_debounce[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////

boolean button_toggle[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int state = 0;//0:일시정지, 1:물받기, 2:샴푸/월풀/스파 3:헹굼 4:욕조청소

void debouncing_button(int i);

void state_0();
void state_1();
void state_2(int i);
void state_3();
void state_4();

void toggle_reset();

void setup() {
  for (int i = 0; i < 9; i++) { //Set button pins as input
    pinMode(button_array[i], INPUT);
  }
  for (int i = 0; i < 6; i++) { //Set button pins as output
    pinMode(led_array[i], OUTPUT);
  }
}

void loop() {
  for (int i = 0; i < 9; i++) { //버튼 받아오기
    debouncing_button[i];
  }
}

void debouncing_button(int i) {
  int reading = digitalRead(button_array[i]);
  if (reading != last_button_state[i])  //스위치의 이전과 지금 상태가 다르면
    last_debounce[i] = millis();   //초를 기록합니다.

  if ((millis() - last_debounce[i]) > debounce_delay) { //스위치 상태가 debounce_delay 이상 같으면
    if (reading != button_state[i]) { //state와 상태가 다를경우 기록
      button_state[i] = reading;
      if (button_state[i] == HIGH) { //HIGH일때 작업 할당
        switch (i) {
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
void state_1() {
  toggle_reset();
  if (state != 1) {// 물받기 킬때
    state = 1;
    button_toggle[0] = 1;
  }
  else {//물받기 끌때

  }
}

void state_2(int i) {
  if (state != 2) {//샴.월.스 처음 킬때
    toggle_reset();
    state = 2;
    button_toggle[i] = 1;
  }
  else {//샴.월.스 토글
    button_toggle[i] = !button_toggle[i];
  }
}

void state_3() {
  toggle_reset();
  if (state != 3) {//헹굼 킬때
    state = 3;
    button_toggle[4] = 1;
  }
  else {//헹굼 끌때

  }
}

void state_4() {
  toggle_reset();
  if (state != 4) {//욕조청소 킬때
    state = 4;
    button_toggle[5] = 1;
  }
  else {//욕조청소 끌때

  }
}

void state_0() {//정지버튼
  toggle_reset();
  state = 0;
  button_toggle[6] = 1;
}

void toggle_reset() { //wash쪽 toggle 리셋
  for (int i = 0; i < 7; i++) {
    button_toggle[i] = 0;
  }
}
