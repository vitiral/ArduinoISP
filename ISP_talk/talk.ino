
SoftwareSerial Talk(MISO, MOSI); // RX, TX
//#define ISP_START_CHAR 0x0A
#define ISP_CHAR1 0x30 // '0'
#define ISP_CHAR2 0x20 // ' '
#define ISP_BUF_CLR() chs[0] = 0; chs[1] = 0

const uint64_t ISP_command = 0x2030;
const uint64_t ISP_2commmand = 0x20302030;

uint8_t entered_avr(char c){
  static char chs[2] = {0, 0};
  static unsigned long time_us[2];
  char ch_i = 0;

  // needs to be valid character
  if(c != ISP_CHAR1 and c != ISP_CHAR2) return 0;

  // see where we are in the array
  if(chs[0] == 0) ch_i = 0;
  else if(chs[1] == 0) ch_i = 1;
  else ch_i = 2;

  switch(ch_i){
  case 0:
    if(c!= ISP_CHAR1){
      ISP_BUF_CLR();
      return 0;
    }
    chs[0] = c;
    time_us[0] = micros();
    return 0;
  case 1:
    if(c != ISP_CHAR2){
      ISP_BUF_CLR();
      return 0;
    }
    chs[1] = c;
    time_us[1] = micros();
    return 0;
  case 2:
    if(c != ISP_CHAR1){
      ISP_BUF_CLR();
    }
    return 1;
  }
  assert_return(0, 0);
}

void talk(){
  wdt_reset();
  char c;
  if(Serial.available()){    
    //if(Serial.peek() == ISP_START_CHAR){
    if(entered_avr(Serial.peek())){
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
  mode = MODE_ISP;
  ;
}




