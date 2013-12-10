
SoftwareSerial Talk(MISO, MOSI); // RX, TX
#define ISP_START_CHAR 0x0A

void talk(){
  char last_c;
  char c;
  if(Serial.available()){
    if(Serial.peek() == ISP_START_CHAR){
      set_mode_isp();
      return;
    }
    c = Serial.read();
    Talk.write(c);
  }
  if(Talk.available()){
    Serial.write(Talk.read());
    Serial.flush();
  }
}

void set_mode_talk(){
  mode = MODE_TALK;
  Talk.begin(57600);
  log_info("Talk Mode:");
  //set_loglevel(LOGV_DEBUG);
}

void set_mode_isp(){
  mode = MODE_ISP;
  //set_loglevel(LOGV_SILENT);
}



