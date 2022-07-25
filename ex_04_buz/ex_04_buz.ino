#define POT A12
#define BUZZER 13

void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t frequency = analogRead(POT);
  tone(BUZZER, frequency);
}
