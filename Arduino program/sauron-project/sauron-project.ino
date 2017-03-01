//David Chatting - david@davidchatting.com
//29th March 2010
//---
// Modified by Rubén Espino

#include "LedControlMS.h"

#define DISPLAY_LED
//#define PROCESSING_INTERFACE

#define compSyncPin 14  //A0  LM1881N:  PIN 1
#define vertSyncPin 15  //A1            PIN 3
#define burstPin    16  //A2            PIN 5
#define oddEvenPin  17  //A3            PIN 7
#define videoPin1   18  //A4
#define videoPin2   19  //A5

#define refVoltagePin1 3
#define refVoltagePin2 5

#define linesInPicture 625  //PAL

LedControl lc=LedControl(12,11,10,1); //DIN, SCK, CS, número de matrices

int numSamplesX = 8;
int numSamplesY = 8;

#define STARTING       0
#define GETTING_X_DIM  1
#define GETTING_Y_DIM  2
#define INITIALIZED    3
int init_state = STARTING;

int lineCount=1;
#define MAX_SAMPLES_X  25
#define MAX_SAMPLES_Y  25
int picture[MAX_SAMPLES_X][MAX_SAMPLES_Y];

int everyNthLine = floor((linesInPicture/2)/numSamplesY);

void setup() {
  int init = 0;
  char inByte = 0;
  
  pinMode(compSyncPin,INPUT);
  pinMode(vertSyncPin,INPUT);
  pinMode(burstPin,INPUT);
  pinMode(oddEvenPin,INPUT);
  
  pinMode(videoPin1,INPUT);
  pinMode(videoPin2,INPUT);
  
  pinMode(refVoltagePin1,OUTPUT);
  pinMode(refVoltagePin2,OUTPUT);
  setRefVoltage(refVoltagePin1, 1.7f);
  setRefVoltage(refVoltagePin2, 3.3f);

#ifdef PROCESSING_INTERFACE
  Serial.begin(115200);

  numSamplesX = 0;
  numSamplesY = 0;

  while(init_state != INITIALIZED)
  {
    if(Serial.available() > 0)
    {
      inByte = Serial.read();
      
      switch(init_state)
      {
        case STARTING:
          if(inByte == 'X')
            init_state = GETTING_X_DIM;
          break;
        case GETTING_X_DIM:
          if(inByte >= '0' && inByte <= '9')
            numSamplesX = numSamplesX*10 + inByte - 48;
          else if(inByte == 'Y' && numSamplesX != 0 && numSamplesX <= MAX_SAMPLES_X)
            init_state = GETTING_Y_DIM;
          else
          {
            if(numSamplesX > MAX_SAMPLES_X)
              Serial.print('E');
            numSamplesX = 0;
            init_state = STARTING;
          }
          break;
        case GETTING_Y_DIM:
          if(inByte >= '0' && inByte <= '9')
            numSamplesY = numSamplesY*10 + inByte - 48;
          else if(inByte == 'S' && numSamplesY != 0 && numSamplesY <= MAX_SAMPLES_Y)
          {
            Serial.print("KKK");
            init_state = INITIALIZED;
          }
          else
          {
            if(numSamplesY > MAX_SAMPLES_Y)
              Serial.print('E');
            numSamplesX = 0;
            numSamplesY = 0;
            init_state = STARTING;
          }
      }
    }
  }
#endif

#ifdef DISPLAY_LED
lc.shutdown(0,false);
lc.setIntensity(0,8);
lc.clearDisplay(0);
#endif
}

void loop(){
  int y=0;
  
  if(!digitalRead(oddEvenPin)){
    while(!digitalRead(oddEvenPin));
    while(!digitalRead(vertSyncPin));
    
    while(digitalRead(oddEvenPin)){
      if((lineCount%everyNthLine)==0){
        //off the shelf analogRead: takes approx 0.1 mS = 100 uS on Decimilla, but 1 line takes 64 uS - boo!
        //hacked version (16) 16 samples in 1000 17ms => 1 sample and write in 17 uS'ish => can do 3 samples of video signal
        //a digital read does 1000 samples and writes in 4mS => 4us per sample => good! => 16 samples
        //line=0;
        
        for(int x=0;x<numSamplesX;++x)
          picture[x][y]=digitalRead(videoPin1) + digitalRead(videoPin2);
          
        ++y;
      }
      ++lineCount;
      
      while(digitalRead(burstPin));
    }
    //this now happens on the odd frame...
    
    setLEDs();
    lineCount=1;
  }
}

void setLEDs(){
#ifdef DISPLAY_LED
  int i, j;
  
  for(int j=0;j<numSamplesY;++j){
    for(int i=0;i<numSamplesX;++i){
      if(picture[i][j] > 0)
        lc.setLed(0,i,j,true);
      else
        lc.setLed(0,i,j,false);
    }
  }
#endif

#ifdef PROCESSING_INTERFACE
  Serial.println('S');

  for(int y=0;y<numSamplesY;++y){
    for(int x=0;x<numSamplesX;++x){
      Serial.print(picture[x][y]);
    }
    Serial.println();
  }
  Serial.println();
#endif
}

void setRefVoltage(int refVoltagePin, float v){
  if(v>=0 && v<=5.0){
    analogWrite(refVoltagePin,(255*v)/5);
    analogWrite(3,(255*v)/5);
  }
}
