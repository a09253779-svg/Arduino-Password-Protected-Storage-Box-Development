#include <EEPROM.h>
#include <Servo.h>
int latchPin=9;
int clockPin=8;
int dataPin=10;

Servo myservo;
int pin_mode=1;      
int pin_key[5]={2,3,4,5,6}; 
int pin_confirm=11;       
int pin_reset=12;        
int pin_voice=7;      
int servo=13;

int key_map[2][5]={
  {0,1,2,3,4}, 
  {5,6,7,8,9} 
};


int hash[2][5]={
  {126,48,109,121,51}, 
  {91,95,112,127,123}  
};

int premode=0;           
bool setting_mode=false;  

int last_mode_state=HIGH;  
unsigned long last_press_time=0;
unsigned long debounce_delay=200;

int input_code[4];     
int input_index=0; 
int password[4]={0,0,0,0};             

void loadPassword(){
  for(int i=0;i<4;i++){
    password[i]=EEPROM.read(i);
    if(password[i] > 9)
      password[i] = 0;
  }
}

void savePassword(){
  for(int i=0;i<4;i++) 
    EEPROM.write(i,password[i]);
}

void setup(){
  pinMode(pin_mode, INPUT_PULLUP);
  

  for(int i = 0; i < 5; i++) 
    pinMode(pin_key[i], INPUT_PULLUP);

  pinMode(pin_confirm, INPUT_PULLUP);
  pinMode(pin_reset, INPUT_PULLUP);

  pinMode(pin_voice, OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  loadPassword(); 
  myservo.attach(servo);
}

void display(int mode, int num){
  if(num>=0&&num<5){
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin,clockPin,LSBFIRST,hash[mode][num]);
    digitalWrite(latchPin,HIGH);
  }
}

void beep(int freq,int duration){
  tone(pin_voice,freq,duration);
  delay(duration);
  noTone(pin_voice);
}

void resetInput(){
  input_index = 0;
}

bool checkPassword(){
  for (int i=0;i<4;i++){
    if (input_code[i]!=password[i]) 
      return false;
  }
  return true;
}

void servo_45_time(){
  myservo.write(120);
  delay(300);  
  myservo.write(90);
}

void loop(){

  int reading=digitalRead(pin_mode);
  if(reading!=last_mode_state){
    if(reading==LOW){
      last_press_time=millis();
    } 
    else if(reading==HIGH){
      unsigned long press_duration=millis()-last_press_time;
      if(press_duration>3000){
        setting_mode=!setting_mode;
        resetInput();
        beep(1200,200);
        beep(1400,200);
      } 
      else if(press_duration>50){ 
        premode=!premode;
        beep(1500,100);
      }
    }
    delay(debounce_delay);
  }
  last_mode_state=reading;

 
  for(int i=0;i<5;i++){
    if(digitalRead(pin_key[i])==LOW){
      if(input_index<4){
        int num_pressed=key_map[premode][i];
        input_code[input_index++]=num_pressed; 
        display(premode,i); 
        beep(1000, 100);
      }
      delay(debounce_delay); 
    }
  }


  if(digitalRead(pin_confirm)==LOW){
    if(input_index==4){
      if(setting_mode){
        for(int i=0;i<4;i++) 
          password[i]=input_code[i];
        savePassword();
        beep(1800,200);
        beep(2000,200);
        setting_mode=false; 
      }
      else{
        if(checkPassword()){
          beep(2000,200);
          beep(2500,200);
          servo_45_time();
        } 
        else{
          beep(500,400);
        }
      }
    }
    resetInput();
    delay(debounce_delay);
  }

  if (digitalRead(pin_reset)==LOW) {
    resetInput();
    beep(800,100);
    delay(debounce_delay);
  }
}
