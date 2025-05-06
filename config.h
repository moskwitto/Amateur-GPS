// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "eeprom_config.h"

//global pin_config
#define IRQ_PIN 2
#define DEBUG_PIN 6
#define OVF_CLOCK_PIN 14
#define CAPTURE_DEBUG_PIN 7
#define DEBUG_CLOCK_PIN 3


// Pin Definitions
#if defined IS_MASTER
  #define IS_MASTER true
  #define CSN_PIN 9
  #define CE_PIN 10
  #define PUSH_BUTTON 15//7
  #define ROLE "Master"
#else
  #define IS_MASTER false
  #define CSN_PIN 10
  #define CE_PIN 9
  #define PUSH_BUTTON 15
  #define ROLE "Slave"
#endif

// Shared Data Structure
struct Message {
    char messageType[5];
    int count;
    unsigned long long  masterCaptureTime;
    unsigned long long  slaveCaptureTime;
    double alpha;
    double beta;
    };

struct NewMessage{
  byte messageType; 
  byte stationID;
  int seqNum;//sequence number
  unsigned long long  captureTime;

}newMessage;


enum class Stage {
  SLEEP,
  MKP,
  MKP_WAIT,
  DATA,
  DATA_WAIT,
}stage;

enum class Channel{
  CHANNEL_78, ///base channel
  CHANNEL_90,
  CHANNEL_100
}channel;

void inline debugPin(bool b){
  //toogle pins
  if(b){
    PORTD|=(1<<PD6);
  }
  else{
    PORTD&=~(1<<PD6);
  }
 }

 void inline captureDebugPin(unsigned long x){
  bool y[32];
  for(int i=0;i<32;i++){
    y[i]=x&(1UL<<i);
  }
  PORTD&=~(1<<PD7);
  for(int i=31;i>=0;i--){
    if(y[i]){
      PORTD|=(1<<PD7);
    }
    else{
      PORTD&=~(1<<PD7);
    }
      PORTD&=~(1<<PD3);
      PORTD|=(1<<PD3);
  }
  PORTD|=(1<<PD7);
 }


void inline ovfClockPin(){
  //toogle pins
    PINC=(1<<PC0);
 }

 
//program eeprom. Address=0;
//Master= Mast1, Mast2....
//slave= Slav1, Slav2....
void writeEEPROM(int addr, const char* data) {
  int length = strlen(data);  // Get actual length of the string
  for (int i = 0; i < length; i++) {
    EEPROM.write(addr + i, data[i]);  // Write each character
  }
  EEPROM.write(addr + length, '\0');  // Ensure null terminator
}

// Function to read a radioID from EEPROM
void readFromEEPROM(int addr, char* buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] = EEPROM.read(addr + i);
        if (buffer[i] == '\0') break;  // Stop at null terminator
    }
}

#endif // CONFIG_H