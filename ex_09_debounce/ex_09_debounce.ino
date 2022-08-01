
#define LED 12
#define BTN_MINUS 34
#define BTN_PLUS 35

bool plusHigh = false, 
     minusHigh = false;
byte ledBrightness = 0;
bool handleClick(byte btn_pin, bool wasHigh, int ledValue ) {
  bool isHigh = digitalRead(btn_pin);
  if (!isHigh && wasHigh) {
    delay(10);
    isHigh = digitalRead(btn_pin);
    if(!isHigh) {
      ledBrightness = constrain(ledBrightness+ledValue, 0, 255);
    }  
  }
  return isHigh;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(12, OUTPUT);
  pinMode(BTN_MINUS, INPUT_PULLUP);
  pinMode(BTN_PLUS, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  plusHigh = handleClick( BTN_PLUS, plusHigh, 35);
  minusHigh = handleClick( BTN_MINUS, minusHigh, -35);
  analogWrite(LED, ledBrightness);
}
