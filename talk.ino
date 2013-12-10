
SoftwareSerial Talk(MISO, MOSI); // RX, TX
#define ISP_START_CHAR 0x0A

void talk(){
  wdt_reset();
  char last_c;
  char c;
  if(Serial.available()){
    if(Serial.peek() == ISP_START_CHAR){
      debug("setting");
      set_mode_isp();
      avrisp();
      return;
    }
    c = Serial.read();
    Talk.write(c);
  }
  if(Talk.available()){
    Serial.write(Talk.read());
  }
  Serial.flush();
}

void set_mode_talk(){
  mode = MODE_TALK;
  Talk.begin(57600);
  set_loglevel(LOGV_DEBUG);
  log_info("Talk Mode:");
}

void set_mode_isp(){
  wdt_reset();
  mode = MODE_ISP;
//  set_loglevel(LOGV_SILENT);
  debug("ISP Mode");
  if(derr) clrerr_log();
}



