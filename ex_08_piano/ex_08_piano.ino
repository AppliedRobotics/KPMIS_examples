#define BUZ 13
#define BTN_FIRST 8
#define DEF_FREQUENCY 3000

void setup() {
  // put your setup code here, to run once:
  for(int i = BTN_FIRST; i < (BTN_FIRST+3); i++) {
  pinMode(i, INPUT);  
  }
  pinMode(BUZ, OUTPUT);
}

void loop() {
  uint16_t frequency = DEF_FREQUENCY;
  // put your main code here, to run repeatedly:
  bool btnPressed = false;
  for (int i = 0; i < 3; i++) {
    if (digitalRead(i+BTN_FIRST)) {
      btnPressed = true;
      frequency += i*500;  
    }
  }
  if ( btnPressed == true ) {
    tone(BUZ, frequency, 50);  
  }
}
