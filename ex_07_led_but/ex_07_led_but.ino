#define BTN 8
#define LED 13
bool state = LOW, memo_state;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool cur_state = digitalRead(BTN);
  if ( cur_state == HIGH && memo_state == LOW ) {
    state = !state;
    digitalWrite(LED, state);
    delay(20);
  }
  memo_state = cur_state;
}
