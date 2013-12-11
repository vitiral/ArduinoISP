//* Written by Garrett Berg, <cloudformdesign.com>, <garrett@cloudformdesign.com>
//* This code is part of the new ArduinoISP code
//* This is the receiver portion, demonstrating how to communicate through
//*  the Arduino programmed with ISP_talk.
//* This code is released into the public domain.

//#define BAUD 57600
#define BAUD 19200
#include "pins_arduino.h"
#include <SoftwareSerial.h>

SoftwareSerial SoftSerial(MOSI, MISO); // RX, TX

void setup(){
  Serial.begin(BAUD); 
  SoftSerial.begin(BAUD);
}

void loop(){
  char c;
  if(SoftSerial.available()) SoftSerial.write(SoftSerial.read());
  if(Serial.available()) Serial.write(Serial.read());
}
