
#include "debug.h"
#include <Serial.h>
#include <Arduino.h>

int derr;
char *errmsg;

const char *DBG_GEN_END_MSG = ")| ";

#define ERR_TYPE          20 // TypeErr
#define ERR_VALUE         21 // ValueErr

#ifdef DEBUG

void DBG_sprintinfo(char *file, unsigned int line){
  sprint(derr); 
  swrite(' '); 
  sprint(file); 
  swrite(':'); 
  sprint(line);
  return;
}

void dbg_start_debug(char *file, unsigned int line){
  sprintln(""); 
  sprint(F("DBG: (")); 
  DBG_sprintinfo(file, line); 
  sprint(DBG_GEN_END_MSG);
}

void dbg_logme(char *file, unsigned int line){
  Serial.println();
  sprint(F("[ERR](")); 
  DBG_sprintinfo(file, line);
  sprint(DBG_GEN_END_MSG); 
  DBG_printerrno();
  Serial.println();
}

void DBG_printerrno(){
  char *strerrno = 0;
  switch(errno){
  case 0:
    strerrno = "NoErr"; break;
  case 1:
    strerrno = "BaseErr"; break;
  case 2:
    strerrno = "TimeoutErr"; break;
  case 3:
    strerrno = "SerialErr"; break;
  case 4:
    strerrno = "SpiErr"; break;
  case 5:
    strerrno = "I2cErr"; break;
  case 6:
    strerrno = "ComErr"; break;
  case 7:
    strerrno = "ConfigErr"; break;
  case 8:
    strerrno = "PinErr"; break;
  case 9:
    strerrno = "InputErr"; break;
  
  case 20:
    strerrno = "TypeErr"; break;
  case 21:
    strerrno = "ValueErr"; break;
  case 22:
    strerrno = "AssertErr"; break;
  
  case 244:
    strerrno = ""; break;
  case 255:
  default:
    strerrno = "UnknownErr:"; break;
  }
  swrite('(');
  sprint(errno);
  swrite(':');
  sprint(strerrno);
  swrite(')');
}

void test(){
  sprint("hello");
}

#endif

