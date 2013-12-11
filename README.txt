ArduinoISP v 2
============

Same as the ArduinoISP (http://arduino.cc/en/Tutorial/ArduinoISP), with
added "Talk" feature.

 New "Talk" Feature:
 After programming, the Arduino passes through all serial communication
   EXCEPT 0x30 (which resets it is an ISP)

 For most arduino boards, configure the software port with:
   SoftwareSerial SoftSerial(MOSI, MISO); // RX, TX
   SoftSerial.begin(57600);
 
 IMPORTANT: your code cannot receive character 0x30 when using this
   library. All other characters are allowed.

