#include "pins_arduino.h"

#include <SoftwareSerial.h>

SoftwareSerial SoftSerial(MOSI, MISO); // RX, TX

void setup(){
  Serial.begin(19200); 
  SoftSerial.begin(57600);
}

void loop(){
  char c;
  if(SoftSerial.available()){
    SoftSerial.write(SoftSerial.read());
  }
    /*
    SoftSerial.print("G:");
    while(SoftSerial.available()){
      c = SoftSerial.read();
      SoftSerial.write(c);
      Serial.write(c);  
    }
    SoftSerial.println("");
    
  }
  */
}
