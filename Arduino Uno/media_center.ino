//media-center // interface with rasbPy
// Jérôme Parisot // 20/12/2013

#include <Wire.h> 
#include "LiquidCrystal_I2C.h"
#include "IRremote.h"
#include "defines.h"
#include "TimeManagement.h"
#include "Potentiometer.h"
#include "utils.h"
#include "SimpleFIFO.h"
#include <string.h>

/* SOFTWARE INITIALISATION */
LiquidCrystal_I2C lcd(LCD_ADRESS);  // set the LCD address to 0x27
int backlight_on = true;

TimeManagement timeM;
unsigned int previousMinute = 0;
char time[6];

Potentiometer potM(POT_GRAY_PIN,POT_BLACK_PIN);

IRrecv irrecv(IR_PIN);
decode_results results;
unsigned long prev_ir_date=0;

boolean power_led_state = false;
unsigned long prev_led_power_switch=0;

State_automate current_state;
State_automate previous_state;
boolean automate_entering_state;
unsigned long automate_state_since;

unsigned char volume=70;
int delta_volume = 0;
int new_volume = false;
int muted = false;
char str_vol[9];

Mode current_mode = M_DEFAULT;
Mode previous_mode = M_DEFAULT;
   
SimpleFIFO<Cmd_RasbPi,10> fifo_cmd_RasbPi;
SimpleFIFO<Cmd,10> fifo_cmd;
boolean time_received = false;
boolean volume_received = false;

uint8_t symb_note2[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t symb_play[8] = {0x10, 0x18, 0x1c, 0x1e, 0x1c, 0x18, 0x10, 0};
uint8_t symb_pause[8] = {0x0, 0x0, 0xa, 0xa, 0xa, 0xa, 0xa, 0x0};
uint8_t symb_radio[8] = {0x4, 0x2, 0x1, 0x1f, 0x1f, 0x1f, 0x0, 0x0};
uint8_t symb_linein[8] = {0x0, 0x4, 0x4, 0xe, 0xe, 0xe, 0x4, 0x2};

uint8_t symb_camera[8] = {0b00000, 0b00000, 0b11110, 0b11111, 0b11111, 0b11110,
        0b01000, 0b10100};

boolean is_video_playing = false;
boolean is_video_paused = false;
TimeManagement videoTime;
char timer_video[9];
unsigned long omx_duration;

char *lines_mode[M_MAX][2][21]; // affichage des lignes 3 et 4 pour chaque mode
boolean new_text = false;

unsigned long int prev_ping_date=0;
unsigned long int prev_ping_date_reception=0;

void setup()
{
  /* 2 -> vcc
     3 -> gnd
     4 -> IR
     12 -> pot_1
     11 -> pot_2
     10 -> gnd
  */
  pinMode(2, OUTPUT);
  digitalWrite(2,HIGH);
  pinMode(3, OUTPUT);
  digitalWrite(3,LOW);
  pinMode(10, OUTPUT);
  digitalWrite(10,LOW);
  
  /* LED POWER */
  pinMode(POWER_LED_PIN,OUTPUT);
  digitalWrite(POWER_LED_PIN,power_led_state);
  
  /* SERIAL */
  Serial.begin(9600);
  /* END SERIAL */
  
  /* TIME */
  timeM.setTime(timeM.timeInSec(0,0,0,0));

  /* END TIME */
  
  /* LCD */
  lcd.begin(20,4);                      // initialize the lcd 20 x 4
  lcd.backlight();
  lcd.clear();
  lcd.createChar(SIMB_NOTE, symb_note2);
  lcd.createChar(SIMB_CAMERA, symb_camera);
  lcd.createChar(SIMB_PLAY, symb_play);
  lcd.createChar(SIMB_PAUSE, symb_pause);
  lcd.createChar(SIMB_RADIO, symb_radio);
  lcd.createChar(SIMB_LINEIN, symb_linein);
  
  lcd.printCentered(1, "Initializing...");
  lcd.printCentered(2, "please wait");
  /* END LCD */
  
  /* IR RECEIVER */
  irrecv.enableIRIn(); // Start the receiver
  /* END IR RECEIVER */
  
  /* AUTOMATE */
  current_state = S_INIT;
  //current_state = S_DEFAULT;
  automate_entering_state = false;
  /* END AUTOMATE */
  
  fifo_cmd.flush();
  fifo_cmd_RasbPi.flush();
  
  lines_mode[M_VIDEO][0];
  lines_mode[M_VIDEO][0][20]=0;
  lines_mode[M_VIDEO][0][0]=0;
  lines_mode[M_VIDEO][1];
  lines_mode[M_VIDEO][1][20]=0;
  lines_mode[M_VIDEO][1][0]=0;
  lines_mode[M_MUSIC][0];
  lines_mode[M_MUSIC][0][20]=0;
  lines_mode[M_MUSIC][0][0]=0;
  lines_mode[M_MUSIC][1];
  lines_mode[M_MUSIC][1][20]=0;
  lines_mode[M_MUSIC][1][0]=0;
}

void loop()
{
  update_volume_from_pot();
  update_IR();
  adjust_to_mode();
  process_rasbPi_serial();
  send_cmd_to_rasbPi();
  
  if( (millis() - prev_ping_date>5000) && (current_state != S_INIT)){
    fifo_cmd_RasbPi.enqueue(C_PING);
    prev_ping_date=millis();
  }
  if((millis() - prev_ping_date_reception>6000) && (current_state != S_INIT)){
    goState(S_INIT);
  }
  
  switch(current_state) {
    
    case S_INIT:
      if(automate_entering_state) {
        automate_entering_state = false;
        
        time_received = false;
        volume_received = false;
        
        lcd.clear();
        lcd.printCentered(1, "Connection lost");
        lcd.printCentered(2, "working on it...");
        lcd.backlight();
      }
      
      while(!time_received || !volume_received) {
        if(!time_received){
          Serial.write(C_RASBPI_GET_TIME); }
        if(!volume_received){
          Serial.write(C_RASBPI_GET_VOLUME); }
        int i;
        for(i=0; i< 4; i++){
          power_led_state = !power_led_state;
          digitalWrite(POWER_LED_PIN,power_led_state);
          delay(500);
        }
        process_rasbPi_serial();
      }
      
      fifo_cmd_RasbPi.enqueue(C_PING);
      prev_ping_date=millis();
      prev_ping_date_reception=millis();
      
      power_led_state = true;
      digitalWrite(POWER_LED_PIN,power_led_state);
      if(backlight_on == false){
        lcd.noBacklight();
      }
      
      goState(S_DEFAULT);
    break;
    
    case S_DEFAULT:
      interpret_cmd_music();
      if(automate_entering_state) {
        automate_entering_state = false;
    	lcd.clear();
        displayTime();
      }
      if (timeM.minute()!=previousMinute){
        previousMinute = timeM.minute();
        displayTime();
          
        if(timeM.minute()%10 == 0){ //time is updated every 10mn (due to the cristal precision)
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_TIME);
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_VOLUME);
        }
      }
      if(strcmp((const char*)lines_mode[M_MUSIC][0], "Stopped")  != 0){
        goState(S_MUSIC);
      }
    break;
    
    case S_MUSIC:
      interpret_cmd_music();
      
      if(automate_entering_state) {
        automate_entering_state = false;
    	lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.write(SIMB_NOTE);
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        lcd.printCentered(2, (char *)lines_mode[M_MUSIC][0]);
        lcd.printCentered(3, (char *)lines_mode[M_MUSIC][1]);
      }
      if (timeM.minute()!=previousMinute){
        previousMinute = timeM.minute();
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        if(timeM.minute()%10 == 0){ //time is updated every 10mn (due to the cristal precision)
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_TIME);
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_VOLUME);
        }
      }
      if(new_text){
        new_text = false;
        lcd.printCentered(2, (char *)lines_mode[M_MUSIC][0]);
        lcd.printCentered(3, (char *)lines_mode[M_MUSIC][1]);
      }
      if(strcmp((const char*)lines_mode[M_MUSIC][0], "Stopped")  == 0){
        goState(S_DEFAULT);
      }
      break;

     case S_LINEIN:
      interpret_cmd_default();
      
      if(automate_entering_state) {
        automate_entering_state = false;
    	lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.write(SIMB_LINEIN);
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        lcd.printCentered(2, "Line Input");
        
        fifo_cmd_RasbPi.enqueue(C_ALSA_LINEIN);
      }
      if (timeM.minute()!=previousMinute){
        previousMinute = timeM.minute();
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        if(timeM.minute()%10 == 0){ //time is updated every 10mn (due to the cristal precision)
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_TIME);
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_VOLUME);
        }
      }
      break;
 
     case S_RADIO:
      interpret_cmd_radio();
      
      if(automate_entering_state) {
        automate_entering_state = false;
    	lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.write(SIMB_RADIO);
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        lcd.printCentered(2, "Radio");
      }
      if (timeM.minute()!=previousMinute){
        previousMinute = timeM.minute();
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        if(timeM.minute()%10 == 0){ //time is updated every 10mn (due to the cristal precision)
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_TIME);
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_VOLUME);
        }
      }
      break;
      
    case S_VIDEO:
      interpret_cmd_video();
      if(automate_entering_state) {
        automate_entering_state = false;
    	lcd.clear();
    
        lcd.setCursor(0,0);
        lcd.write(SIMB_CAMERA);
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        if (is_video_playing){
          lcd.printCentered(2, (char *)lines_mode[M_VIDEO][0]);
          //lcd.setCursor(8,3);
          //sprintf(timer_video, "%02d:%02d", videoTime.hour(), videoTime.minute());
          //lcd.print(timer_video);
        } else {
          lcd.printCentered(3," PC isn't streaming");
        }
      }
      if (timeM.minute()!=previousMinute){
        previousMinute = timeM.minute();
        sprintf(time, "%02d:%02d", timeM.hour(), timeM.minute());
        lcd.setCursor(15,0);
        lcd.print(time);
        
        if(timeM.minute()%10 == 0){ //time is updated every 10mn (due to the cristal precision)
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_TIME);
          fifo_cmd_RasbPi.enqueue(C_RASBPI_GET_VOLUME);
        }
      }
      if (is_video_playing){
        lcd.setCursor(5,3);
        if(is_video_paused){
          lcd.write(SIMB_PAUSE);
        } else {
          lcd.write(SIMB_PLAY);
        }
      }
      break;
      
    case S_VOLUME:
      if(automate_entering_state) {
        automate_entering_state = false;
        reset_time_state();
        new_volume = false;
        
        lcd.clear();
        lcd.printCentered(1,"Volume:");
                
        if(!muted){
         sprintf(str_vol,"%3d%%", volume);
         lcd.printCentered(2,str_vol);
        } else {
          lcd.printCentered(2,"Mute on");
        }
        
      } else if (new_volume) {
        new_volume = false;
        reset_time_state();
        
        if(!muted){
          lcd.setCursor(5,2);
          sprintf(str_vol,"  %3d%%  ", volume);
          lcd.print(str_vol);
        } else {
          lcd.printCentered(2,"Mute on");
        }
        
      }
      if(is_time_state_over(1500)){
        goState(previous_state);
      }
    break;
    
    case S_VOLUME_OMX:
      if(automate_entering_state) {
        automate_entering_state = false;
        reset_time_state();
        new_volume = false;
        
        lcd.clearLine(3);
        
        if(delta_volume > 0){
          lcd.printCentered(1,"Volume:");
          lcd.printCentered(2,"up");
        } else {
          lcd.printCentered(1,"Volume:");
          lcd.printCentered(2,"down");
        }
        
      } else if (new_volume) {
        new_volume = false;
        reset_time_state();
        
        if(delta_volume > 0){
          lcd.printCentered(1,"Volume:");
          lcd.printCentered(2,"up");
        } else {
          lcd.printCentered(1,"Volume:");
          lcd.printCentered(2,"down");
        }
        
      }
      if(is_time_state_over(1500)){
        goState(previous_state);
      }
    break;
  }
}

void displayTime(){
  char first_line[20];
  sprintf(first_line, "%02d : %02d", timeM.hour(), timeM.minute());
  char second_line[20];
  sprintf(second_line, "%s", timeM.day());
  
  lcd.printCentered(1, first_line);
  lcd.printCentered(2, second_line);
}

void goState(State_automate s){
  previous_state = current_state;
  current_state = s;
  automate_entering_state = true;
}

void reset_time_state(){
  automate_state_since=millis();
}
int is_time_state_over(unsigned int milli){
  return( (millis()-automate_state_since)>milli?true:false);
}

void update_volume_from_pot(){
  if ( current_state > S_INIT){
    potM.update();
    delta_volume = potM.get_delta();
    
    if (delta_volume != 0){
      change_volume(delta_volume);
    }
  }
}

void update_IR(){
  while(irrecv.decode(&results)) {
    irrecv.resume(); // Receive the next value
    
    if(millis()-prev_ir_date>200){
      prev_ir_date = millis();
      
      switch(results.value) {
        case _PLUS:
         change_volume(1);
         break;
        case _MINUS:
         change_volume(-1);
         break;
        case _SLEEP:
         switch_mute();
         break;
        case _TUNER_MEM:
         fifo_cmd.enqueue(CMD_TUNER_MEM);
         break;
        case _DISPLAY:
         if (backlight_on){
           backlight_on = false;
           lcd.noBacklight();
         } else {
           backlight_on = true;
           lcd.backlight();
         }
         break;
         case _FUNCTION:
           if(current_mode != M_VIDEO){
             fifo_cmd.flush();
             current_mode = M_VIDEO;
             fifo_cmd_RasbPi.enqueue(C_ALSA_LINEIN_OFF);
           }
         break;
         case _TAPE:
           if(current_mode != M_MUSIC){
             fifo_cmd.flush();
             current_mode = M_MUSIC;
             fifo_cmd_RasbPi.enqueue(C_ALSA_LINEIN_OFF);
           }
           break;
         case _CD:
           if(current_mode != M_LINEIN){
             fifo_cmd.flush();
             current_mode = M_LINEIN;
             fifo_cmd_RasbPi.enqueue(C_MPD_STOP);
             fifo_cmd_RasbPi.enqueue(C_ALSA_LINEIN);
           }
           break;
         case _TUNER:
           if(current_mode != M_RADIO){
             fifo_cmd.flush();
             current_mode = M_RADIO;
             fifo_cmd_RasbPi.enqueue(C_MPD_STOP);
             fifo_cmd_RasbPi.enqueue(C_ALSA_LINEIN);
           }
           break;
         case _PREVIOUS:
           fifo_cmd.enqueue(CMD_PREVIOUS);
           break;
         case _NEXT:
           fifo_cmd.enqueue(CMD_NEXT);
           break;
         case _BACK:
           fifo_cmd.enqueue(CMD_BACK);
           break;
         case _FORWARD:
           fifo_cmd.enqueue(CMD_FORWARD);
           break;
         case _PLAY:
           fifo_cmd.enqueue(CMD_PLAY);
           break;
         case _PAUSE:
           fifo_cmd.enqueue(CMD_PAUSE);
           break;
         case _STOP:
           fifo_cmd.enqueue(CMD_STOP);
           break;
         case _CLEAR:
           fifo_cmd.enqueue(CMD_CLEAR);
           break;
         case _ENTER:
           fifo_cmd.enqueue(CMD_ENTER);
           break;
         case _ALBUM_PLUS:
           fifo_cmd.enqueue(CMD_ALBUM_PLUS);
           break;
         case _ALBUM_MINUS:
           fifo_cmd.enqueue(CMD_ALBUM_MINUS);
           break;
           
         default:
         //do nothing
         break;
      }
    }
  }
}

void change_volume(int i){
  new_volume = true;
  if((current_state != S_VIDEO) && (current_state != S_VOLUME_OMX)){
    volume += i*2;
    if( volume >200) {volume=0;} //uint 8
    else if (volume >100) {volume = 100;}
    if (current_state != S_VOLUME) {
      goState(S_VOLUME);
    }
    muted = false;
    fifo_cmd_RasbPi.enqueue(C_ALSA_VOLUME);
  } else {
    if (current_state != S_VOLUME_OMX) {
      goState(S_VOLUME_OMX);
    }
    delta_volume = i;
    if (i > 0){
      fifo_cmd_RasbPi.enqueue(C_OMX_V_PLUS);
    } else {
      fifo_cmd_RasbPi.enqueue(C_OMX_V_MINUS);
    }
  }
}

void switch_mute(){
  new_volume = true;
  muted = muted?false:true;
  if (current_state != S_VOLUME) {
    goState(S_VOLUME);
  }
  lcd.clearLine(2);
  fifo_cmd_RasbPi.enqueue(C_ALSA_MUTE);
}

void adjust_to_mode(){
  if (current_state != S_INIT && current_state != S_VOLUME && current_state != S_VOLUME_OMX) {
    switch(current_mode){
      case M_DEFAULT:
        if( (current_state != S_MUSIC) && (current_state != S_DEFAULT) ){
          goState(S_DEFAULT);
        }
        break;
      case M_MUSIC:
        if( (current_state != S_MUSIC) && (current_state != S_DEFAULT) ){
          goState(S_MUSIC);
        }
        break;
      case M_VIDEO:
        if( current_state != S_VIDEO){
          goState(S_VIDEO);
        }
        break;
      case M_RADIO:
        if( current_state != S_RADIO){
          goState(S_RADIO);
        }
        break;
      case M_LINEIN:
        if( current_state != S_LINEIN){
          goState(S_LINEIN);
        }
        break;
    }
  }
}

void interpret_cmd_music() {
  while(fifo_cmd.count()>0){
    Cmd cmd = fifo_cmd.dequeue();
    switch(cmd){
      case CMD_PREVIOUS:
      fifo_cmd_RasbPi.enqueue(C_MPD_PREVIOUS);
      break;
      case CMD_NEXT:
      fifo_cmd_RasbPi.enqueue(C_MPD_NEXT);
      break;
      case CMD_BACK:
      fifo_cmd_RasbPi.enqueue(C_MPD_BACK);
      break;
      case CMD_FORWARD:
      fifo_cmd_RasbPi.enqueue(C_MPD_FORWARD);
      break;
      case CMD_PLAY:
      fifo_cmd_RasbPi.enqueue(C_MPD_PLAY);
      break;
      case CMD_PAUSE:
      fifo_cmd_RasbPi.enqueue(C_MPD_PAUSE);
      break;
      case CMD_STOP:
      fifo_cmd_RasbPi.enqueue(C_MPD_STOP);
      break;
      case CMD_CLEAR:
      fifo_cmd_RasbPi.enqueue(C_MPD_CLEAR);
      break;
      case CMD_TUNER_MEM:
      fifo_cmd_RasbPi.enqueue(C_MPD_LAUNCH_RADIO);
      break;
      default:
      //do nothing
      break;
    }
  }
}

void interpret_cmd_video() {
  while(fifo_cmd.count()>0){
    Cmd cmd = fifo_cmd.dequeue();
    switch(cmd){
      case CMD_PREVIOUS:
      fifo_cmd_RasbPi.enqueue(C_OMX_PREVIOUS);
      break;
      case CMD_NEXT:
      fifo_cmd_RasbPi.enqueue(C_OMX_NEXT);
      break;
      case CMD_BACK:
      fifo_cmd_RasbPi.enqueue(C_OMX_BACK);
      break;
      case CMD_FORWARD:
      fifo_cmd_RasbPi.enqueue(C_OMX_FORWARD);
      break;
      case CMD_PLAY:
      fifo_cmd_RasbPi.enqueue(C_OMX_PLAY);
      is_video_paused = false;
      break;
      case CMD_PAUSE:
      fifo_cmd_RasbPi.enqueue(C_OMX_PAUSE);
      is_video_paused=!is_video_paused;
      break;
      case CMD_STOP:
      fifo_cmd_RasbPi.enqueue(C_OMX_STOP);
      is_video_playing=false;
      videoTime.setTime(0);
      break;
      case CMD_ALBUM_MINUS:
      fifo_cmd_RasbPi.enqueue(C_OMX_SUB_MINUS);
      break;
      case CMD_ALBUM_PLUS:
      fifo_cmd_RasbPi.enqueue(C_OMX_SUB_PLUS);
      break;
      default:
      //do nothing
      break;
    }
  }
}

void interpret_cmd_radio() {
  while(fifo_cmd.count()>0){
    Cmd cmd = fifo_cmd.dequeue();
    switch(cmd){ /* TODO */
	/*
      case CMD_PREVIOUS:
      fifo_cmd_RasbPi.enqueue(C_MPD_PREVIOUS);
      break;
      case CMD_NEXT:
      fifo_cmd_RasbPi.enqueue(C_MPD_NEXT);
      break;
      case CMD_BACK:
      fifo_cmd_RasbPi.enqueue(C_MPD_BACK);
      break;
      case CMD_FORWARD:
      fifo_cmd_RasbPi.enqueue(C_MPD_FORWARD);
      break;
      case CMD_PLAY:
      fifo_cmd_RasbPi.enqueue(C_MPD_PLAY);
      break;
      case CMD_PAUSE:
      fifo_cmd_RasbPi.enqueue(C_MPD_PAUSE);
      break;
      case CMD_STOP:
      fifo_cmd_RasbPi.enqueue(C_MPD_STOP);
      break;
      case CMD_CLEAR:
      fifo_cmd_RasbPi.enqueue(C_MPD_CLEAR);
      break;
      case CMD_TUNER_MEM:
      fifo_cmd_RasbPi.enqueue(C_MPD_LAUNCH_RADIO);
      break;
      */
      default:
      //do nothing
      break;
    }
  }
}

void interpret_cmd_default() {
  while(fifo_cmd.count()>0){
    Cmd cmd = fifo_cmd.dequeue();
  }
}

void send_cmd_to_rasbPi(){
  while(fifo_cmd_RasbPi.count()>0){
    Cmd_RasbPi cmd = fifo_cmd_RasbPi.dequeue();
    Serial.write(cmd);
    if(cmd == C_ALSA_VOLUME){
      Serial.write(volume);
    }
  }
}

void process_rasbPi_serial(){
  Input_rasbPi in_rp;
  int size_buff = Serial.available();
  char line;
  
  while(size_buff>0){
    in_rp = (Input_rasbPi)Serial.read();
    size_buff--;
    
    switch(in_rp){
      case INP_TIME:
        unsigned long time_in_sec;
        Serial.readBytes(TO_CHAR_POINTER(time_in_sec),4);
        size_buff-=4;
        timeM.setTime(time_in_sec);
        time_received = true;
        break;
      case INP_VOLUME:
        Serial.readBytes(TO_CHAR_POINTER(volume),1);
        size_buff--;
        volume_received = true;
        break;
      case INP_TEXT:
        Serial.readBytes(&line,1);
        char text[21];
        text[20]=0;
        Serial.readBytes(text,20);
        lcd.printCentered(line,text);
        break;
      case INP_MODE:
        previous_mode = current_mode;
        Serial.readBytes(TO_CHAR_POINTER(current_mode),1);
        if (current_mode == M_VIDEO){
          is_video_playing = true;
          is_video_paused = false;
          lcd.backlight();
          backlight_on = true;
        }
        break;
      case INP_END_MODE:
        if (current_mode == M_VIDEO){
          is_video_playing = false;
          lines_mode[M_VIDEO][0][0]=0;
        }
        current_mode = previous_mode;
        break;
      case INP_LINE_MODE:
        char mode;
        Serial.readBytes(&mode,1);
        Serial.readBytes(&line,1);
        Serial.readBytes((char *)lines_mode[mode][line-3],20);
        new_text = true;
        break;
      case INP_OMX_DURATION:
        Serial.readBytes(TO_CHAR_POINTER(omx_duration),4);
        videoTime.setTime(omx_duration);
        break;
      case INP_PING_BACK:
        prev_ping_date_reception = millis();
        break;
      default:
      //do nothing
      break;
    }
  }
}
