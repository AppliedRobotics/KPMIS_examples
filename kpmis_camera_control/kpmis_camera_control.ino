#include "DxlMaster.h"
#include "Servo.h"

#define SERIAL_BAUD 115200

#define DXL_BAUD 1000000
#define DXL_START_REG 6
#define NUM_REG_PAIRS 6
#define DXL_DEV_ID 51
DynamixelDevice Device(DXL_DEV_ID);
int16_t data[NUM_REG_PAIRS];


Servo servo_X;  // перемещение по X
Servo servo_Y;  // перемещение по Y
Servo servo_Z;  // перемещение по Z
Servo servo_G;  // схват

volatile float pwm[4] = {90, 90, 90, 90};
static float pwm_min[4] = {30, 30, 30, 70};
static float pwm_max[4] = {180, 180, 180, 140};

void setup() {
  Serial.begin(SERIAL_BAUD);
  DxlMaster.begin(DXL_BAUD);
  Device.init();
  pinMode(22, OUTPUT);  
  Serial.println("Initiated");
  pinMode(44, OUTPUT); // motors
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(47, OUTPUT);
  
  servo_X.attach(9);   // перемещение по X
  servo_Y.attach(10);  // перемещение по Y
  servo_Z.attach(11);  // перемещение по Z
  servo_G.attach(12);  // перемещение по G
}

typedef struct
{
  int16_t man_x;
  int16_t man_y;
  int16_t move_x;
  int16_t move_y;
  int16_t man_z;
  int16_t grip;
}JoystickData_t;


void loop() {
  JoystickData_t joy;
  DxlMaster.read(51,6, 12, (uint8_t*)(&joy));
  
  int vl = (-joy.move_x + joy.move_y) / 2 / 4;
  int vr = (joy.move_x + joy.move_y) / 2 / 4;
  //if(joy.move_x < 0 & joy.move_y > 0 ){
  analogWrite(44, abs(vl));//влево
  digitalWrite(45, (vl > 0) ? LOW : HIGH);
  analogWrite(46, abs(vr));
  digitalWrite(47, (vr > 0) ? LOW : HIGH);
   
  Serial.print("man_x "   +   String(joy.man_x) + " ");
  Serial.print("man_y "   +   String(joy.man_y) + " ");
  Serial.print("move_x "  +   String(joy.move_x) + " ");
  Serial.print("move_y "  +   String(joy.move_y) + " ");
  Serial.print("man_z "   +   String(joy.man_z) + " ");
  Serial.print("grip "    +   String(joy.grip) + "\n");
  delay(50);

  if (abs(joy.man_x) < 1001) pwm[0] -= joy.man_x / 250;
  if (abs(joy.man_y) < 1001) pwm[1] -= joy.man_y / 250;
  if (abs(joy.man_y) < 1001) pwm[2] -= joy.man_z / 250;
  if (joy.grip == 0) 
    pwm[3] = pwm_max[3];
  if (joy.grip == 1) 
    pwm[3] = pwm_min[3];

  for (int i = 0; i < 4; i++) {
    if (pwm[i] > pwm_max[i])
      pwm[i] = pwm_max[i];
    if (pwm[i] < pwm_min[i])
      pwm[i] = pwm_min[i];
  }
  
  servo_X.write((int)pwm[0]); // перемещение по X
  servo_Y.write((int)pwm[1]); // перемещение по Y
  servo_Z.write((int)pwm[2]); // перемещение по Z
  servo_G.write((int)pwm[3]);

  Serial.print("servo_X "   +   String(pwm[0]) + " ");
  Serial.print("servo_Y "   +   String(pwm[1]) + " ");
  Serial.print("servo_Z "   +   String(pwm[2]) + " ");
  Serial.print("servo_G "   +   String(pwm[3]) + "\n");
}
