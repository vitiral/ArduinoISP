
// ArduinoISP version 04m3
// Copyright (c) 2008-2011 Randall Bohn
// If you require a license, see
// http://www.opensource.org/licenses/bsd-license.php
//
// This sketch turns the Arduino into a AVRISP
// using the following arduino pins:
//
// pin name: not-mega: mega(1280 and 2560)
// slave reset: 10: 53
// MOSI: 11: 51
// MISO: 12: 50
// SCK: 13: 52
//
// Put an LED (with resistor) on the following pins:
// 9: Heartbeat - shows the programmer is running
// 8: error - Lights up if something goes wrong (use red if that makes sense)
// 7: Programming - In communication with the slave
//
// 23 July 2011 Randall Bohn
// -Address Arduino issue 509 :: Portability of ArduinoISP
// http://code.google.com/p/arduino/issues/detail?id=509
//
// October 2010 by Randall Bohn
// - Write to EEPROM > 256 bytes
// - Better use of LEDs:
// -- Flash LED_PMODE on each flash commit
// -- Flash LED_PMODE while writing EEPROM (both give visual feedback of writing progress)
// - Light LED_ERR whenever we hit a STK_NOSYNC. Turn it off when back in sync.
// - Use pins_arduino.h (should also work on Arduino Mega)
//
// October 2009 by David A. Mellis
// - Added support for the read signature command
//
// February 2009 by Randall Bohn
// - Added support for writing to EEPROM (what took so long?)
// Windows users should consider WinAVR's avrdude instead of the
// avrdude included with Arduino software.
//
// January 2008 by Randall Bohn
// - Thanks to Amplificar for helping me with the STK500 protocol
// - The AVRISP/STK500 (mk I) protocol is used in the arduino bootloader
// - The SPI functions herein were developed for the AVR910_ARD programmer
// - More information at http://code.google.com/p/mega-isp

// When burning using the ISP, the computer send out three xA signals.
// for the UNO you need a 10uF cap between reset and gnd
// In boards.txt, the target board needs to have :
//   BOARD.upload.using=avrisp
//   BOARD.upload.protocol=avrisp

#include "SPI.h"
#include "pins_arduino.h"
#include "debug.h"

#include <SoftwareSerial.h>

#define RESET SS

#define LED_HB 9
#define LED_ERR 8
#define LED_PMODE 7
#define PROG_FLICKER true

#define HWVER 2
#define SWMAJ 1
#define SWMIN 18

// STK Definitions
#define STK_OK 0x10
#define STK_FAILED 0x11
#define STK_UNKNOWN 0x12
#define STK_INSYNC 0x14
#define STK_NOSYNC 0x15
#define CRC_EOP 0x20 //ok it is a space...

int error = 0;
void pulse(int pin, int times);


#define TALK_MODE 1
#define ISP_MODE 2
char mode;
SoftwareSerial SoftSerial(MISO, MOSI); // RX, TX


// Debug stuff
#define MAX_STR_BUF 30
char strbuffer[MAX_STR_BUF + 2];
int str_i = 0;

void setup() {
  strbuffer[0] = 0;
  Serial.begin(9600);
  pinMode(LED_PMODE, OUTPUT);
  pulse(LED_PMODE, 2);
  pinMode(LED_ERR, OUTPUT);
  pulse(LED_ERR, 2);
  pinMode(LED_HB, OUTPUT);
  pulse(LED_HB, 2);

  setup_talk();
  debug("Starting...");
  stderr = 0;
}

void setup_isp(){
  SoftSerial.end();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  // Clock Div can be 2,4,8,16,32,64, or 128
  SPI.setClockDivider(SPI_CLOCK_DIV128);

  digitalWrite(LED_PMODE, LOW);
  digitalWrite(LED_ERR, LOW);
  digitalWrite(LED_HB, LOW);
  mode = ISP_MODE;
}

void setup_talk(){
  SPI.end();
  SoftSerial.begin(57600);

  digitalWrite(LED_PMODE, HIGH);
  digitalWrite(LED_ERR, HIGH);
  digitalWrite(LED_HB, HIGH);

  mode = TALK_MODE;
}

int pmode=0;
// address for reading and writing, set by 'U' command
int here;
uint8_t buff[256]; // global block storage

#define beget16(addr) (*addr * 256 + *(addr+1) )
typedef struct param {
  uint8_t devicecode;
  uint8_t revision;
  uint8_t progtype;
  uint8_t parmode;
  uint8_t polling;
  uint8_t selftimed;
  uint8_t lockbytes;
  uint8_t fusebytes;
  uint8_t flashpoll;
  uint16_t eeprompoll;
  uint16_t pagesize;
  uint16_t eepromsize;
  uint32_t flashsize;
}
parameter;

parameter param;

// this provides a heartbeat on pin 9, so you can tell the software is running.
uint8_t hbval=128;
int8_t hbdelta=8;
void heartbeat() {
  if (hbval > 192) hbdelta = -hbdelta;
  if (hbval < 32) hbdelta = -hbdelta;
  hbval += hbdelta;
  analogWrite(LED_HB, hbval);
  delay(20);
}


char entered_isp = false;
long int isp_time = 0;
void loop(void) {
  // set to talk mode if no transmission has been sent in 250 ms
  static long int last_print = millis();
  
  assert((mode == TALK_MODE) or ((millis() - isp_time < 250)));
  noerr();

  short int no_serial;
  if(Serial.available()){
    if(str_i != MAX_STR_BUF){
      strbuffer[str_i] = Serial.peek();
      strbuffer[str_i + 1] = 0;
      str_i++;
    }
    no_serial = false;
  }
  else{
    no_serial = true;
  }

  if(mode == ISP_MODE){
    printerrno();
    clearerror();
    // is pmode active?
    if (pmode) digitalWrite(LED_PMODE, HIGH);
    else digitalWrite(LED_PMODE, LOW);
    // is there an error?
    if (error) digitalWrite(LED_ERR, HIGH);
    else digitalWrite(LED_ERR, LOW);

    // light the heartbeat LED
    heartbeat();
    if(no_serial) return;
    if (Serial.available()) {
      avrisp();
    }
  }
  else if(mode == TALK_MODE){
    if(millis() - last_print > 2000){
      Serial.println("");
      Serial.print(entered_isp, HEX);
      Serial.print(" ");
      Serial.print(str_i);
      Serial.print(" Rec:");
      Serial.print(strbuffer);
      Serial.print(" H: ");
      for(int n = 0; n<str_i; n++){
        Serial.print("x");
        Serial.print(strbuffer[n], HEX);
        Serial.print(" ");
      }
      last_print = millis();
    }
    if(no_serial) return;
    talk();
  }
  return;
error:
  if(derr == ERR_TIMEOUT){  
    if(errno != 0) errno = ERR_TIMEOUT;
    log_err("loop T");
    clearerror();
    mode = TALK_MODE;
  }
  else{
    log_err("Unknown");
    clearerror();
    mode = TALK_MODE;
  }
}

void talk(){
  //transmits data through SoftSerial. A value of 0xA resets it.
  //TODO: need 0xA three times?
  char c;
  if(Serial.available()){
    c = Serial.peek();
    if(c == 0xA){
      setup_isp();
      avrisp();
      return;
    }
    c = Serial.read();
    SoftSerial.write(c);
  }
  if(SoftSerial.available()){
    Serial.write(SoftSerial.read());
  }
}

uint8_t getch() {
  while(!Serial.available()){
    assert(millis() - isp_time < 250);
  }
  return Serial.read();
error:
  errno = ERR_TIMEOUT;
  log_err("getch TO");
  return -1;
}

void fill(int n) {
  for (int x = 0; x < n; x++) {
    buff[x] = getch();
    noerr_log();
    }
error:
    return;
}

#define PTIME 30
void pulse(int pin, int times) {
  do {
    digitalWrite(pin, HIGH);
    delay(PTIME);
    digitalWrite(pin, LOW);
    delay(PTIME);
  }
  while (times--);
}

void prog_lamp(int state) {
  if (PROG_FLICKER)
    digitalWrite(LED_PMODE, state);
}

uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint8_t n;
  SPI.transfer(a);
  n=SPI.transfer(b);
  //if (n != a) error = -1;
  n=SPI.transfer(c);
  return SPI.transfer(d);
}

void empty_reply() {
  char c = getch();
  noerr_log();
  if (CRC_EOP == c) {
    Serial.print((char)STK_INSYNC);
    Serial.print((char)STK_OK);
  }
  else {
    error++;
    Serial.print((char)STK_NOSYNC);
  }
error:
  return;
}

void breply(uint8_t b) {
  char c = getch();
  noerr_log();
  if (CRC_EOP == c) {
    Serial.print((char)STK_INSYNC);
    Serial.print((char)b);
    Serial.print((char)STK_OK);
  }
  else {
    error++;
    Serial.print((char)STK_NOSYNC);
  }
error:
  return;
}

void get_version(uint8_t c) {
  switch(c) {
  case 0x80:
    breply(HWVER);
    break;
  case 0x81:
    breply(SWMAJ);
    break;
  case 0x82:
    breply(SWMIN);
    break;
  case 0x93:
    breply('S'); // serial programmer
    break;
  default:
    breply(0);
  }
}

void set_parameters() {
  // call this after reading paramter packet into buff[]
  param.devicecode = buff[0];
  param.revision = buff[1];
  param.progtype = buff[2];
  param.parmode = buff[3];
  param.polling = buff[4];
  param.selftimed = buff[5];
  param.lockbytes = buff[6];
  param.fusebytes = buff[7];
  param.flashpoll = buff[8];
  // ignore buff[9] (= buff[8])
  // following are 16 bits (big endian)
  param.eeprompoll = beget16(&buff[10]);
  param.pagesize = beget16(&buff[12]);
  param.eepromsize = beget16(&buff[14]);

  // 32 bits flashsize (big endian)
  param.flashsize = buff[16] * 0x01000000
    + buff[17] * 0x00010000
    + buff[18] * 0x00000100
    + buff[19];

}

void start_pmode() {
  SPI.begin();
  digitalWrite(RESET, HIGH);
  pinMode(RESET, OUTPUT);
  digitalWrite(SCK, LOW);
  delay(20);
  digitalWrite(RESET, LOW);
  spi_transaction(0xAC, 0x53, 0x00, 0x00);
  pmode = 1;
}

void end_pmode() {
  SPI.end();
  digitalWrite(RESET, HIGH);
  pinMode(RESET, INPUT);
  pmode = 0;
}

void universal() {
  int w;
  uint8_t ch;

  fill(4);
  noerr_log();
  if(error) return;
  ch = spi_transaction(buff[0], buff[1], buff[2], buff[3]);
  breply(ch);
error:
  return;
}

void flash(uint8_t hilo, int addr, uint8_t data) {
  spi_transaction(0x40+8*hilo,
  addr>>8 & 0xFF,
  addr & 0xFF,
  data);
}
void commit(int addr) {
  if (PROG_FLICKER) prog_lamp(LOW);
  spi_transaction(0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0);
  if (PROG_FLICKER) {
    delay(PTIME);
    prog_lamp(HIGH);
  }
}

//#define _current_page(x) (here & 0xFFFFE0)
int current_page(int addr) {
  if (param.pagesize == 32) return here & 0xFFFFFFF0;
  if (param.pagesize == 64) return here & 0xFFFFFFE0;
  if (param.pagesize == 128) return here & 0xFFFFFFC0;
  if (param.pagesize == 256) return here & 0xFFFFFF80;
  return here;
}

void write_flash(int length) {
  fill(length);
  char c = getch();
  noerr_log();
  if (CRC_EOP == c) {
    Serial.print((char) STK_INSYNC);
    Serial.print((char) write_flash_pages(length));
  }
  else {
    error++;
    Serial.print((char) STK_NOSYNC);
  }
error:
  return;
}

uint8_t write_flash_pages(int length) {
  int x = 0;
  int page = current_page(here);
  while (x < length) {
    if (page != current_page(here)) {
      commit(page);
      page = current_page(here);
    }
    flash(LOW, here, buff[x++]);
    flash(HIGH, here, buff[x++]);
    here++;
  }

  commit(page);

  return STK_OK;
}

#define EECHUNK (32)
uint8_t write_eeprom(int length) {
  // here is a word address, get the byte address
  int start = here * 2;
  int remaining = length;
  if (length > param.eepromsize) {
    error++;
    return STK_FAILED;
  }
  while (remaining > EECHUNK) {
    write_eeprom_chunk(start, EECHUNK);
    noerr_log();
    start += EECHUNK;
    remaining -= EECHUNK;
  }
  write_eeprom_chunk(start, remaining);
  noerr_log()
    return STK_OK;
error:
  return STK_FAILED;
}
// write (length) bytes, (start) is a byte address
uint8_t write_eeprom_chunk(int start, int length) {
  // this writes byte-by-byte,
  // page writing may be faster (4 bytes at a time)
  fill(length);
  noerr_log();
  prog_lamp(LOW);
  for (int x = 0; x < length; x++) {
    int addr = start+x;
    spi_transaction(0xC0, (addr>>8) & 0xFF, addr & 0xFF, buff[x]);
    delay(45);
  }
  prog_lamp(HIGH);
  return STK_OK;
error:
  return STK_FAILED;
}

void program_page() {
  char c;
  char result = (char) STK_FAILED;
  int length = 256 * getch();
  length += getch();
  char memtype = getch();
  noerr_log();
  // flash memory @here, (length) bytes
  if (memtype == 'F') {
    write_flash(length);
    noerr_log();
    return;
  }
  if (memtype == 'E') {
    result = (char)write_eeprom(length);
    c = getch();
    noerr_log();
    if (CRC_EOP == c) {
      Serial.print((char) STK_INSYNC);
      Serial.print(result);
    }
    else {
      error++;
      Serial.print((char) STK_NOSYNC);
    }
    return;
  }
  Serial.print((char)STK_FAILED);
  return;
error:
  return;
}

uint8_t flash_read(uint8_t hilo, int addr) {
  return spi_transaction(0x20 + hilo * 8,
  (addr >> 8) & 0xFF,
  addr & 0xFF,
  0);
}

char flash_read_page(int length) {
  for (int x = 0; x < length; x+=2) {
    uint8_t low = flash_read(LOW, here);
    Serial.print((char) low);
    uint8_t high = flash_read(HIGH, here);
    Serial.print((char) high);
    here++;
  }
  return STK_OK;
}

char eeprom_read_page(int length) {
  // here again we have a word address
  int start = here * 2;
  for (int x = 0; x < length; x++) {
    int addr = start + x;
    uint8_t ee = spi_transaction(0xA0, (addr >> 8) & 0xFF, addr & 0xFF, 0xFF);
    Serial.print((char) ee);
  }
  return STK_OK;
}

void read_page() {
  char result = (char)STK_FAILED;
  int length = 256 * getch();
  length += getch();
  char memtype = getch();
  char c = getch();
  noerr_log();
  if (CRC_EOP != c) {
    error++;
    Serial.print((char) STK_NOSYNC);
    return;
  }
  Serial.print((char) STK_INSYNC);
  if (memtype == 'F') result = flash_read_page(length);
  if (memtype == 'E') result = eeprom_read_page(length);
  Serial.print(result);
  return;
error:
  return;
}

void read_signature() {
  uint8_t high, middle, low;
  char c = getch();
  noerr_log();
  if (CRC_EOP != c) {
    error++;
    Serial.print((char) STK_NOSYNC);
    return;
  }
  Serial.print((char) STK_INSYNC);
  high = spi_transaction(0x30, 0x00, 0x00, 0x00);
  Serial.print((char) high);
  middle = spi_transaction(0x30, 0x00, 0x01, 0x00);
  Serial.print((char) middle);
  low = spi_transaction(0x30, 0x00, 0x02, 0x00);
  Serial.print((char) low);
  Serial.print((char) STK_OK);
  return;
error:
  return;
}
//////////////////////////////////////////
//////////////////////////////////////////


////////////////////////////////////
////////////////////////////////////
int avrisp() {
  entered_isp = true;
  uint8_t data, low, high;
  char c;
  uint8_t ch = getch();
  noerr_log();
  isp_time = millis();
  switch (ch) {
  case '0': // signon
    error = 0;
    empty_reply();
    break;
  case '1':
    c = getch();
    noerr_log();
    if (c == CRC_EOP) {
      Serial.print((char) STK_INSYNC);
      Serial.print("AVR ISP");
      Serial.print((char) STK_OK);
    } 
    else {
      error++;
      Serial.print((char) STK_NOSYNC);
    }
    break;
  case 'A':
    c = getch();
    noerr_log();
    get_version(c);
    break;
  case 'B':
    fill(20);
    noerr_log();
    set_parameters();
    empty_reply();
    break;
  case 'E': // extended parameters - ignore for now
    fill(5);
    noerr_log();
    empty_reply();
    break;

  case 'P':
    if (pmode) {
      pulse(LED_ERR, 3);
    } 
    else {
      start_pmode();
    }
    noerr_log();
    empty_reply();
    break;
  case 'U': // set address (word)
    here = getch();
    here += 256 * getch();
    noerr_log();
    empty_reply();
    break;

  case 0x60: //STK_PROG_FLASH
    low = getch();
    high = getch();
    noerr_log();
    empty_reply();
    break;
  case 0x61: //STK_PROG_DATA
    data = getch();
    noerr_log();
    empty_reply();
    break;

  case 0x64: //STK_PROG_PAGE
    program_page();
    break;

  case 0x74: //STK_READ_PAGE 't'
    read_page();
    break;
  case 'V': //0x56
    universal();
    break;
  case 'Q': //0x51
    error=0;
    end_pmode();
    noerr_log();
    empty_reply();
    break;

  case 0x75: //STK_READ_SIGN 'u'
    read_signature();
    break;

    // expecting a command, not CRC_EOP
    // this is how we can get back in sync
  case CRC_EOP:
    error++;
    Serial.print((char) STK_NOSYNC);
    break;

    // anything else we will return STK_UNKNOWN
  default:
    error++;
    c = getch();
    noerr_log();
    if (CRC_EOP == c)
      Serial.print((char)STK_UNKNOWN);
    else
      Serial.print((char)STK_NOSYNC);
  }
  noerr_log();
  return 0;
error:
  return 1;
}




















