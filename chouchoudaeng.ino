boolean dryer_power[2] = {0, 0};
int button_array[9] = {22, 23, 24, 25, 26, 27, 28, 29, 30};
int led_array[6] = {40, 41, 42, 43, 44, 45};

///////these lines are for debouncing buttons///////////
boolean last_button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
boolean button_state[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int last_debounce[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////

boolean button_toggle[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

void debouncing_button(int i);

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
      if (button_state[i] == HIGH) //HIGH일때 작업 할당
        button_toggle[i] = ~button_toggle[i];
    }
  }
  last_button_state[i] = reading;
}
