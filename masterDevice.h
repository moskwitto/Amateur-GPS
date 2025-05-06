#include "Print.h"
#ifndef MASTERDEVICE_H
#define MASTERDEVICE_H

#include "NRF24Radio.h"
#include "utils.h"

class MasterDevice {
private:
  NRF24Radio& radio;
  long int MKPCaptureTime;

public:
  MasterDevice(NRF24Radio& radio)
    : radio(radio) {}
  unsigned long timeout=2000;
  unsigned long startListenTime;
  
  void masterLoop2(){
    while(message.count<pktCount){
      transmitLoop();
    }
    radio.startListening();
    message.count=0;
    messageCount=0;
    n=0;
    Serial.println("listening");
    
    while(message.count<pktCount){
      receiveLoop();
    }
    
     messageCount=0;
      m=0;
      n=0;
      sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0,sumY2=0;
      radio.stopListening();
      snprintf(message.messageType, sizeof(message.messageType), "DATA");
      message.count=0;
      message.masterCaptureTime = slaveTime;//masterEndTime
      message.slaveCaptureTime = firstSlaveCaptureTime;//master start time
      message.alpha=previous.alpha;//alpha_s
      message.beta=beta;
      delay(500);
      radio.stopListening();
      // delay(2000);
      radio.sendMessage(message);
      radio.stopListening();
     
 
  }
  // void superMaster(){
  //   superMasterLoop(); 
  // }
};

#endif  // MASTERDEVICE_H