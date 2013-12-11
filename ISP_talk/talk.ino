
SoftwareSerial Talk(MISO, MOSI); // RX, TX
//#define ISP_START_CHAR 0x0A
#define ISP_START_CHAR 0x30

void talk(){
  wdt_reset();
  char last_c;
  char c;
  if(Serial.available()){
    if(Serial.peek() == ISP_START_CHAR){
      set_mode_isp();
      avrisp();
      return;
    }
    c = Serial.read();
    debug(String("from_user:") + c);
    Talk.write(c);
  }
  
  if(Talk.available()){
    c = Talk.read();
    debug(String("from_ard:") + c);
    Serial.write(c);
  }
  
  Serial.flush();
}

void set_mode_talk(){
  mode = MODE_TALK;
  Talk.begin(57600);
  Serial.println("Talk Mode:");
}

void set_mode_isp(){
  debug("ISP Mode");
  Talk.end();
  wdt_reset();
  mode = MODE_ISP;;
}



