#include "NRF24Radio.h"
#include "lcd.h"
#include "rotary_encoder.h"

// Radio and device objects
NRF24Radio radio(CE_PIN, CSN_PIN);
#if IS_MASTER
#include "masterDevice.h"
// Addresses for communication
const byte address[5] = "MYGPS";
const byte addressAck[5] = "YOGPS";
MasterDevice master(radio);
#else
#include "slaveDevice.h"
// Addresses for communication
const byte addressAck[5] = "MYGPS";
const byte address[5] = "YOGPS";
SlaveDevice slave(radio);
#endif

void setup() {
    Serial.begin(115200);
    Serial.print("OCR1A ");
    Serial.println(OCR1A);
    Serial.print("TCCR1A ");
    Serial.println(TCCR1A);
    Serial.print("TIMSK1 ");
    Serial.println(TIMSK1);
    
    radio.initialize(address, addressAck);
    Serial.print("Initialized as: ");
    Serial.println(ROLE);

    if(EEPROM_PROGRAMMING_MODE){
      //  char write[6]="Slav1";
      // char write[6]="Mast1";
      // char write[6]="Mast2";
      // char write[6]="Mast3";
      //  writeEEPROM(0,write);
    }

    readFromEEPROM(0,radio.radioID,sizeof(radio.radioID));
    Serial.println(radio.radioID);

    if(strcmp(radio.radioID,"Slav1")==0){
      radio.setChannel(90);
    }
    else if(strcmp(radio.radioID,"Mast1")==0){
      radio.setChannel(90);
    }
    else if(strcmp(radio.radioID,"Mast2")==0){
      radio.setChannel(100);
    }
    else if(strcmp(radio.radioID,"Mast3")==0){
      radio.setChannel(90); //100
    }
    else{
      Serial.println("Error reading EEPROM!");
      while(1){}
    }
    //rotaryEncoderSetup();
    delay(1000);
    radio.printDetails();

    #if LCD_ON
      //lcdSetup();
    #endif   

    // stage=Stage::SLEEP;
    radio.flushBuffer();
    // radio.startListening();
    #if IS_MASTER
      armed=true;
      Message message;
      snprintf(message.messageType, sizeof(message.messageType), "DATA1");
      message.count=0;
      message.masterCaptureTime=totalCaptureTime;
      message.slaveCaptureTime=0;
      radio.stopListening();
      radio.sendMessage(message);
    #else
      armed=true;
      radio.flushBuffer();
      radio.startListening();
    #endif
}

void loop() {
#if IS_MASTER
  master.masterLoop2();
  // master.superMaster();
#else
// if(!isListening){
//     buttonStatus();
//     rotaryStatus();
//    lcdUpdateSelection();
// }

  // if(isListening){
    slave.slaveLoop();
  // }
#endif
}
