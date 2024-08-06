/** Arduino Dynamixel blobs example.
 * Settings: Blob detector, UART half duplex, addr 51, Dynamixel API, 5V.
 * Wiring:
 *   Camera 3-pin Dynamixel to Arduino 3-pin Dynamixel   
 */

#include "TrackingCamDxl.h"

TrackingCamDxl trackingCam(51);
unsigned long previousMillis = 0; // stores last time cam was updated

void setup() {
  // Init Dynamixel interface
  DxlMaster.begin(1000000);
  Serial.begin(115200);
  delay(5000);
}

void loop() {
  uint8_t n = trackingCam.readBlobs(5); // read data about first 5 blobs
  Serial.println("All blobs");
  Serial.println(n); // print numbers of blobs
  for(int i = 0; i < n; i++) // print information about all blobs
  {
    Serial.print(trackingCam.blob[i].type, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].dummy, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].cx, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].cy, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].area, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].left, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].right, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].top, DEC);
    Serial.print(" ");
    Serial.print(trackingCam.blob[i].bottom, DEC);
    Serial.println(" ");
  }
  
  // wait for the next frame
  while(millis() - previousMillis < 33) 
  {};
  previousMillis = millis();
}


