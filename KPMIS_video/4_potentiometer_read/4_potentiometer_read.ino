#define PotPin A10 // Пин потенциометра
void setup() {
  Serial.begin(115200); //Инициализируем терминал
  pinMode(PotPin, INPUT); //Обьявляем пин потенциометра на считывание
}

void loop() {
  Serial.print("Potentiometer value: ");//Начинаем писать строку не прерывая её
  Serial.println(analogRead(PotPin)); // дописываем строку, выводя считанные с потенциометра данные
  delay(100); 
}
