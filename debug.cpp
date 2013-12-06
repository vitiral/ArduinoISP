
#include "debug.h"
#include <Serial.h>
#include <Arduino.h>

int derr;
char *errmsg;

char *strerrno(int err){
  switch(err){
  case 0:
    return "NoError:";
  case 1:
    return "AssertionErr:";
  case 2:
    return "TimeoutErr:";
  case 255:
  default:
    return "UnknownErr:";
  }
}

void printerrno(){
  if(errno){
    Serial.print(strerrno(errno)); 
    Serial.println(errmsg);
  }
}

void test(){
  sprint("hello");
}


