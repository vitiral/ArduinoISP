

SoftwareSerial Talk(MISO, MOSI); // RX, TX
//#define ISP_START_CHAR 0x0A
#define ISP_CHAR1 0x30 // '0'
#define ISP_CHAR2 0x20 // ' '
#define ISP_BUF_CLR() chs[0] = 0; chs[1] = 0

const uint64_t ISP_command = 0x2030;
const uint64_t ISP_2commmand = 0x20302030;

char Read(){
  char c = Serial.read();
  Talk.write(c);
  //debug(String("from_user:x") + String((uint8_t)c, HEX) + String("-t\t") + micros());
  return c;
}

uint8_t entered_avr(){
  unsigned long time;
  uint8_t ind = 0;

  if( Read() != ISP_CHAR1){
    return 0;
  }
  delayMicroseconds(800);
  assert_raise_return(Serial.available(), ERR_TIMEOUT, 0);
  assert_raise_return(Read() == ISP_CHAR2, ERR_VALUE, 0);
  
  // If anything is sent before the time interval, invalid
  time = millis();
  while(millis() - time < 245){
    wdt_reset();
    assert_raisem_return(!Serial.available(), ERR_TIMEOUT, 
      String("too soon ") + String(millis() - time), 0);
  }
  
  time = millis();
  while(millis() - time < 10){
    // this is the correct time window to get data
    wdt_reset();
    if(Serial.available()){
      if(Serial.peek() == ISP_CHAR1){
        return 1; // it is an isp call
      }
      else{
        raise_return(ERR_TIMEOUT, 0);
      }
    }
  }
  
 return 0;
}

void talk(){
  wdt_reset();
  char c;
  if(Serial.available()){    
    //if(Serial.peek() == ISP_START_CHAR){
    if(entered_avr()){ // does reading
      set_mode_isp();
      avrisp();
      return;
    }
  }

  if(Talk.available()){
    c = Talk.read();
    //debug(String("from_ard:") + c);
    Serial.write(c);
  }
  
  //TODO: I need to flush and simultaniously pet the dog, but there
  //  is no way that I can tell to check if there is data in the
  //  buffer
  //Serial.flush();
}

void set_mode_talk(){
  mode = MODE_TALK;
  Talk.begin(SOFT_BAUD);
  Serial.println("Talk Mode:");
}

void set_mode_isp(){
  //debug("ISP Mode");
  Talk.end();
  wdt_reset();
  mode = MODE_ISP;
  ;
}




