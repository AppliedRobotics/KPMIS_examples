// Объявляем необходимые библиотеки
#include <PID_v1.h>
#include "Servo.h"
#include "TrackingCamDxl2.h"
#include "DxlMaster2.h"

// Выставляем скорость
#define DXLBAUD 115200

// Объявляем пины кнопок на плате
#define BLOB_BUTTON 34   // 1 кнопка выполняет подпрограмму управления с помощью Потенциометр
#define ARUCO_BUTTON 32  // 2 кнопка выполняет подпрограмму слежения за Аруко маркером
#define test_BUTTON 30   

// Объявление Мультидатчика
#define LFS_CTRL_MEASURE_START_STOP_REG 24
#define LFS_SENSOR_EN_REG 60
#define LFS_DATA_REG 80
#define LFS_TEMP 130
DynamixelDevice lfs(202);
uint16_t data_from_lfs = 0;

// Объявляем УЗ-Дальномер
#define echoPin 4
#define trigPin 5

// Объявляем пины Моторов
#define M1_D 45
#define M1_P 44
#define M2_D 47
#define M2_P 46
#define MAX_SPEED 40        // Максимальная скорость для колес
#define MIN_SPEED 15        // Минимальная скорость для колес
uint8_t SPEED = MAX_SPEED;  // Создаем переменные для корректировки скорости

// Пины для датчиков скорости (выбраны 18 и 19, так как они поддерживают прерывания на Arduino Mega)
#define SENSOR_PIN_LEFT 2
#define SENSOR_PIN_RIGHT 3

// Константы для расчёта скорости
#define PULSES_PER_REV 20   // Количество импульсов за оборот (зависит от вашего датчика)
#define INTERVAL_MS 100     // Интервал для расчёта скорости (мс)

// Переменные для подсчёта импульсов
volatile unsigned long pulseCountLeft = 0;
volatile unsigned long pulseCountRight = 0;

// Переменные для PID
double setpointLeft, setpointRight;   // Желаемая скорость (RPM)
double inputLeft, inputRight;         // Текущая скорость (RPM)
double outputLeft, outputRight;       // Выход ШИМ

// Создание PID-регуляторов
PID pidLeft(&inputLeft, &outputLeft, &setpointLeft, 0.05, 0.01, 0.05, DIRECT);
PID pidRight(&inputRight, &outputRight, &setpointRight, 0.07, 0.01, 0.07, DIRECT);

Servo servo[5];
static uint8_t motorPins[2][2] = { { M1_D, M1_P }, { M2_D, M2_P } };                        
static uint8_t servoPins[5] = {7, 8, 9, 10, 11};                                    
static uint8_t limits[5][2] = { { 30, 150 }, { 30, 150 }, { 30, 150 }, { 30, 150 }, { 30, 150 } };  
static uint8_t ledPins[6] = { 22, 23, 24, 25, 26, 27 };                             

TrackingCamDxl2 cam(2, 2);  // Объявляем камеру
int duration, Distance;
int j = 0;
uint8_t application = 0, lastApp = 0;
uint8_t myBlob = 99;

void setup() {
  // Подключаем Сервоприводы
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(servoPins[i], OUTPUT);
    servo[i].attach(servoPins[i]);
  }

  // Подключаем Моторы и двигаем для проверки
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 2; j++) {
      pinMode(motorPins[i][j], OUTPUT); // Исправлено с 1 на OUTPUT
    }
    checkMoveMotors(i);
  }

  // Подключаем светодиоды
  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Подаем сигнал на УЗ-Дальномер
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Настройка датчиков скорости
  pinMode(SENSOR_PIN_LEFT, INPUT);
  pinMode(SENSOR_PIN_RIGHT, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN_LEFT), countPulseLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN_RIGHT), countPulseRight, RISING);

  // Инициализация PID
  pidLeft.SetMode(AUTOMATIC);
  pidRight.SetMode(AUTOMATIC);
  pidLeft.SetOutputLimits(0, 255);
  pidRight.SetOutputLimits(0, 255);
  // Начальные параметры PID (Kp=1.0, Ki=0.0, Kd=0.0) заданы выше, их нужно будет настроить

  Serial.begin(DXLBAUD);
  Serial2.begin(DXLBAUD);
  DxlMaster.begin(DXLBAUD);
  DxlMaster.setTimeOut(50);
  lfs.protocolVersion(2);
  initLFS();        // Инициализация мультидатчика
  application = 0;  // Выставляем подпрограмму в 0 при первом запуске / перезагрузке
}

void loop() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= INTERVAL_MS) {
    calculateSpeed(); // Расчёт скорости, но без активации PID
    lastTime = millis();
  }

  getDataRange();
  checkButtonsPressed();
  if (lastApp != application) changeApp(application);
  switch (application) {
    case 1:
      if (j == 0) {
        followBlobs(0);
      } else if (j == 1) {
        followBlobs(1);
      } else {
        while (application == 1) {
          checkButtonsPressed();
          digitalWrite(motorPins[0][0], LOW);
          digitalWrite(motorPins[1][0], HIGH);
          analogWrite(motorPins[0][1], 0);
          analogWrite(motorPins[1][1], 0);
        }
      }
      break;
    case 2:
      followAruco();
      break;
    case 3:
      if (j == 0) {
        test(0);
      } else if (j == 1) {
        test(1);
      } else {
        while (application == 3) {
          checkButtonsPressed();
          digitalWrite(motorPins[0][0], LOW);
          digitalWrite(motorPins[1][0], HIGH);
          analogWrite(motorPins[0][1], 0);
          analogWrite(motorPins[1][1], 0);
        }
      }
      break;
    default:
      break;
  }
}

// Функция для расчёта скорости (не активирует PID, только обновляет input)
void calculateSpeed() {
  noInterrupts();
  unsigned long pulsesLeft = pulseCountLeft;
  unsigned long pulsesRight = pulseCountRight;
  pulseCountLeft = 0;
  pulseCountRight = 0;
  interrupts();

  // Расчёт RPM (оборотов в минуту)
  inputLeft = (pulsesLeft / (double)PULSES_PER_REV) * (60000.0 / INTERVAL_MS);
  inputRight = (pulsesRight / (double)PULSES_PER_REV) * (60000.0 / INTERVAL_MS);

  // Отладочный вывод
  Serial.print("inputLeft: "); Serial.print(inputLeft);
  Serial.print(" setpointLeft: "); Serial.print(setpointLeft);
  Serial.print(" outputLeft: "); Serial.println(outputLeft);
  Serial.print("inputRight: "); Serial.print(inputRight);
  Serial.print(" setpointRight: "); Serial.print(setpointRight);
  Serial.print(" outputRight: "); Serial.println(outputRight);
}

// Функции для прерываний
void countPulseLeft() {
  pulseCountLeft++;
}

void countPulseRight() {
  pulseCountRight++;
}

// Функция небольшого сдвига Моторов при запуске платы
void checkMoveMotors(uint8_t i) {
  digitalWrite(motorPins[i][0], LOW);
  analogWrite(motorPins[i][1], 40);
  delay(200);
  digitalWrite(motorPins[i][0], HIGH);
  analogWrite(motorPins[i][1], 40);
  delay(200);
  digitalWrite(motorPins[i][0], LOW);
  analogWrite(motorPins[i][1], 0);
  delay(200);
}

// Функция проверки нажатой кнопки
void checkButtonsPressed() {
  if (digitalRead(BLOB_BUTTON) == HIGH) application = 1;
  if (digitalRead(ARUCO_BUTTON) == HIGH) application = 2;
  if (digitalRead(test_BUTTON) == HIGH) application = 3;
}

// Первая подпрограмма слежения за цветом
void followBlobs(int a) {
  digitalWrite(motorPins[0][0], LOW);
  digitalWrite(motorPins[1][0], HIGH);
  cam.readBlobs(2);
  uint16_t sensors_en_buf[10] = { 0 };
  for (uint8_t q = 0; q < 2; q++) {
    if (cam.blob[q].type == a) {
      myBlob = q;
    }
  }
  int dx = 320 - cam.blob[myBlob].cx;
  uint32_t area = cam.blob[myBlob].area;
  Serial.println(cam.blob[myBlob].type);
  Serial.print("cx: " + String(cam.blob[myBlob].cx));
  Serial.print(" ");
  Serial.print("cy: " + String(cam.blob[myBlob].cy));
  Serial.println();
  Serial.print("left: " + String(cam.blob[myBlob].left));
  Serial.print(" ");
  Serial.print("right: " + String(cam.blob[myBlob].right));
  Serial.print(" ");
  Serial.print("top: " + String(cam.blob[myBlob].top));
  Serial.print(" ");
  Serial.print("bottom: " + String(cam.blob[myBlob].bottom));
  Serial.print(" ");
  Serial.print("area: " + String(cam.blob[myBlob].area));
  Serial.println();
  Serial.println(j);
  Serial.println();
  Serial.println(myBlob);
  uint32_t close = 18000;
  int pwm = map(abs(dx), 30, 320, 40, 70);
  if (area > 1 && area < close) {
    if (dx > 40 && dx < 320) {
      pwm += 10;
      analogWrite(motorPins[0][1], 0);
      analogWrite(motorPins[1][1], pwm);
    } else if (dx < -40) {
      pwm += 10;
      analogWrite(motorPins[0][1], pwm);
      analogWrite(motorPins[1][1], 0);
    } else {
      pwm = 60;
      analogWrite(motorPins[0][1], pwm);
      analogWrite(motorPins[1][1], pwm);
    }
  }else if (area >= close && area < 591041808) {
    digitalWrite(motorPins[0][0], LOW);
    digitalWrite(motorPins[1][0], HIGH);
    analogWrite(motorPins[0][1], 0);
    analogWrite(motorPins[1][1], 0);
    for (int i = 110; i >= 70; i--) {
      servo[1].write(i);
      delay(15);
    }
    for (int i = 80; i <= 120; i++) {
      servo[2].write(i);
      delay(15);
    }
    Serial.print("blob ");
    Serial.println(a);
    for (int i = 0; i < 3; i++) {
      lfs.read(LFS_DATA_REG, data_from_lfs);
      sendToEsp(String(data_from_lfs));
      Serial.print("temp: ");
      Serial.println(data_from_lfs / 10);
      delay(1000);
    }
    for (int i = 120; i >= 80; i--) {
      servo[2].write(i);
      delay(15);
    }
    for (int i = 70; i <= 110; i++) {
      servo[1].write(i);
      delay(15);
    }
    digitalWrite(motorPins[0][0], HIGH);
    digitalWrite(motorPins[1][0], LOW);
    analogWrite(motorPins[0][1], 50);
    analogWrite(motorPins[1][1], 50);
    delay(3000);
    j++;
  } else {
    digitalWrite(motorPins[0][0], LOW);
    digitalWrite(motorPins[1][0], HIGH);
    analogWrite(motorPins[0][1], 0);
    analogWrite(motorPins[1][1], 0);
  }
  delay(50);
}

// Вторая подпрограмма слежение за кубиком Аруко
void followAruco() {
  digitalWrite(motorPins[0][0], LOW);
  digitalWrite(motorPins[1][0], HIGH);
  cam.readAruco(1);
  uint16_t cx = 0;
  DxlMaster.read(2, 2, 107, cx);
  int dx = 320 - cx;
  int close1 = 15;
  Serial.println(dx);
  int pwm = map(abs(dx), 30, 320, 40, 60);
  Serial.println(pwm);
  if (Distance > 1 && Distance > close1) {
    if (dx > 40 && dx < 320) {
      pwm += 10;
      analogWrite(motorPins[0][1], 0);
      analogWrite(motorPins[1][1], pwm);
    } else if (dx < -40) {
      pwm += 10;
      analogWrite(motorPins[0][1], pwm);
      analogWrite(motorPins[1][1], 0);
    } else {
      pwm = 60;
      analogWrite(motorPins[0][1], pwm);
      analogWrite(motorPins[1][1], pwm);
    }
  } else if (Distance <= close1) {
    digitalWrite(motorPins[0][0], LOW);
    digitalWrite(motorPins[1][0], HIGH);
    analogWrite(motorPins[0][1], 0);
    analogWrite(motorPins[1][1], 0);
    delay(500);
    for (int i = 110; i >= 70; i--) {
      servo[1].write(i);
      delay(15);
    }
    for (int i = 80; i <= 120; i++) {
      servo[2].write(i);
      delay(15);
    }
    while (application == 2) {
      checkButtonsPressed();
      lfs.read(LFS_DATA_REG, data_from_lfs);
      sendToEsp(String(data_from_lfs));
      Serial.print("temp: ");
      Serial.println(data_from_lfs / 10);
      delay(200);
    }
  } else {
    digitalWrite(motorPins[0][0], LOW);
    digitalWrite(motorPins[1][0], HIGH);
    analogWrite(motorPins[0][1], 0);
    analogWrite(motorPins[1][1], 0);
  }
}

void test(int a) {
  digitalWrite(motorPins[0][0], LOW);  // Направление левого мотора
  digitalWrite(motorPins[1][0], HIGH); // Направление правого мотора
  cam.readBlobs(2);                    // Чтение данных с камеры
  for (uint8_t q = 0; q < 2; q++) {
    if (cam.blob[q].type == a) {
      myBlob = q;                      // Выбор нужного объекта
    }
  }
  int dx = 320 - cam.blob[myBlob].cx;    // Смещение от центра
  uint32_t area = cam.blob[myBlob].area; // Площадь объекта

  // Вывод данных для отладки
  Serial.println(cam.blob[myBlob].type);
  Serial.print("cx: " + String(cam.blob[myBlob].cx));
  Serial.print(" cy: " + String(cam.blob[myBlob].cy));
  Serial.println();
  Serial.print("area: " + String(area));
  Serial.println();

  uint32_t close = 18000; // Порог для остановки
  int baseRPM = map(abs(dx), 30, 320, MIN_SPEED, MAX_SPEED); // Базовая скорость

  if (area > 1 && area < close) {
    if (dx > 40 && dx < 320) {        // Поворот влево
      setpointLeft = 0;               // Остановка левого мотора
      setpointRight = baseRPM + 50;   // Ускорение правого мотора
    } else if (dx < -40) {            // Поворот вправо
      setpointLeft = baseRPM + 50;    // Ускорение левого мотора
      setpointRight = 0;              // Остановка правого мотора
    } else {                          // Движение вперёд
      setpointLeft = 2000;            // Скорость 2000 RPM
      setpointRight = 2000;
    }
    // Вычисление PID
    pidLeft.Compute();
    pidRight.Compute();
    // Применение к моторам
    analogWrite(motorPins[0][1], outputLeft);
    analogWrite(motorPins[1][1], outputRight);
  } else if (area >= close) {
    // Остановка через PID
    setpointLeft = 0;
    setpointRight = 0;
    pidLeft.Compute();
    pidRight.Compute();
    analogWrite(motorPins[0][1], outputLeft);
    analogWrite(motorPins[1][1], outputRight);
  } else {
    // Полная остановка
    analogWrite(motorPins[0][1], 0);
    analogWrite(motorPins[1][1], 0);
  }
  delay(50); // Задержка для плавности
}

// Функция отправления данных на ESP32
void sendToEsp(String value) {
  String str = String(value);
  char c;
  for (uint8_t i = 0; i < str.length(); i++) {
    c = str[i];
    Serial2.write(c);
    delay(50);
  }
  delay(700);
}

// Функция считывания данных с УЗ-Дальномера
void getDataRange() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  Distance = duration / 58;
  delay(50);
}

// Функция получения данных по шине dxl
void getData() {
  cam.readBlobs(2);
  Serial.print("cx: " + String(cam.blob[1].cx));
  Serial.print(" ");
  Serial.print("cy: " + String(cam.blob[1].cy));
  Serial.println();
  Serial.print("left: " + String(cam.blob[1].left));
  Serial.print(" ");
  Serial.print("right: " + String(cam.blob[1].right));
  Serial.print(" ");
  Serial.print("top: " + String(cam.blob[1].top));
  Serial.print(" ");
  Serial.print("bottom: " + String(cam.blob[1].bottom));
  Serial.println();
}

void initLFS() {
  uint16_t sensors_en_buf[10] = { 0 };
  lfs.write(LFS_CTRL_MEASURE_START_STOP_REG, 0);
  delay(500);
  lfs.write(LFS_SENSOR_EN_REG, 20, (uint8_t*)sensors_en_buf);
  delay(500);
  sensors_en_buf[0] = LFS_TEMP;
  lfs.write(LFS_SENSOR_EN_REG, 20, (uint8_t*)sensors_en_buf);
  delay(500);
  lfs.write(LFS_CTRL_MEASURE_START_STOP_REG, 1);
  delay(500);
}

// Функция переключения светодиода на плате
void ledChange(uint8_t key) {
  for (int i = 0; i < 6; i++) {
    if (i + 1 == key) digitalWrite(ledPins[i], 1);
    else digitalWrite(ledPins[i], 0);
  }
}

// Функция смены подпрограммы
void changeApp(uint8_t app) {
  checkMoveMotors(0);
  checkMoveMotors(1);
  ledChange(app);
  switch (app) {
    case 1:
      j = 0;
      lastApp = 1;
      break;
    case 2:
      servo[0].write(95);
      servo[1].write(110);
      servo[2].write(80);
      servo[3].write(100);
      for (int i = 100; i >= 30; i--) {
        servo[4].write(i);
        delay(15);
      }
      lastApp = 2;
      break;
    case 3:
      j = 0;
      lastApp = 3;
      break;
    default:
      lastApp = 6;
      break;
  }
}