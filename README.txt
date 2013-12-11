ArduinoISP v 2
============

Same as the ArduinoISP (http://arduino.cc/en/Tutorial/ArduinoISP), with
added "Talk" feature.

To use:
 Open and load ISP_talk onto your arduino, follow the directions in the
    tutorial linked above to connect and program the board
 To program your target device select the correct board and set the
    programmer to "Arduino as ISP"
 Press "Cntrl+Shift+U" to upload your sketch (instead of just "Cntrl+U")
 
 Follow directions below to take advantage of the talk feature.

 New "Talk" Feature: 
 After programming, the Arduino passes through all serial communication
   (except the note below)

 For most arduino boards, configure the software port with:
   SoftwareSerial SoftSerial(MOSI, MISO); // RX, TX
   SoftSerial.begin(19200); // ISP_talk communicates at 19200 baud
 
 IMPORTANT: ISP mode is triggered when the following is received from the computer
	:: 0x30, 0x20, (delay 250ms +/- 5ms), 0x30, 0x20, (delay 250ms +/- 5ms), 0x30
	Where "0x30, 0x20" are consecutive bits (within 800us of eachother)
	Note that 0x30 == '0' and 0x20 == ' '
	
	The Arduino can send any characters.

  Check out ISP_receiver for a simple example of how to use this library
   (it simply returns everything it receives)
