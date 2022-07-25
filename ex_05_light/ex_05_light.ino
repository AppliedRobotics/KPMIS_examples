#define POT A0
#define LIG A1
#define LED 13

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (analogRead(POT) < analogRead(LIG)) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);  
  }
}
