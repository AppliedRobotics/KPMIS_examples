#define LED1 22 //Объявляем пины светодиодов от 22 до 27, присваивая, соответственно, имена от LED1 до LED6
#define LED2 23
#define LED3 24
#define LED4 25
#define LED5 26
#define LED6 27

void setup() {
  /**
   * Инициализируем пины светодиодов на выход сигнала.
   */
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  }

void loop() {
  digitalWrite(LED6, LOW); //Выключаем 6 светодиод
  digitalWrite(LED1, HIGH);//Включаем 1 светодиод
  delay(100); //Пауза в 100 миллисекунд
  digitalWrite(LED1, LOW); //Выключаем 1 светодиод
  digitalWrite(LED2, HIGH);//Включаем 2 светодиод
  delay(100); //Пауза в 100 миллисекунд
  digitalWrite(LED2, LOW); //Выключаем 2 светодиод
  digitalWrite(LED3, HIGH);//Включаем 3 светодиод
  delay(100); //Пауза в 100 миллисекунд
  digitalWrite(LED3, LOW); //Выключаем 3 светодиод
  digitalWrite(LED4, HIGH);//Включаем 4 светодиод
  delay(100); //Пауза в 100 миллисекунд
  digitalWrite(LED4, LOW); //Выключаем 4 светодиод
  digitalWrite(LED5, HIGH);//Включаем 5 светодиод
  delay(100); //Пауза в 100 миллисекунд
  digitalWrite(LED5, LOW); //Выключаем 5 светодиод
  digitalWrite(LED6, HIGH);//Включаем 6 светодиод
  delay(100); //Пауза в 100 миллисекунд
}
