#define LedPin 22 //Пин светодиода
void setup() {
  pinMode(LedPin, OUTPUT); //Обявление светодиода на вывод
}

void loop() {
  digitalWrite(LedPin,HIGH); //Включение светодиода
  delay(500); //Пауза
  digitalWrite(LedPin,LOW); //Выключение светодиода
  delay(500); //Пауза
}
