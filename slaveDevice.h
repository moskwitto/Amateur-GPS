#ifndef SLAVEDEVICE_H
#define SLAVEDEVICE_H

#include "NRF24Radio.h"
#include "utils.h"

struct lcdData {
    float distance_1 = 0.0;
    float distance_2 = 0.0;
    float distance_3 = 0.0;

    double alpha_1=0.0;
    double alpha_2=0.0;
    double alpha_3=0.0;

    float beta_1 = 0.0;
    float beta_2 = 0.0;
    float beta_3 = 0.0;

    int channel_1;
    int channel_2;
    int channel_3;

    int currentChannel;
} lcdData;

class SlaveDevice {
private:
    NRF24Radio& radio;
    long int MKPCaptureTime;

public:
    SlaveDevice(NRF24Radio& radio) : radio(radio) {}

   void slaveLoop() {
      messageCount=0;
      message.count=0;
      m=0;
      sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0,n=0,sumY2=0.0;
      while(message.count<pktCount){
        receiveLoop();
      }
      message.count=0;
      messageCount=0;
      radio.stopListening();
      snprintf(message.messageType, sizeof(message.messageType), "DATA");
      message.count=0;
      message.masterCaptureTime = previous.slaveCaptureTime;//slaveEndTime
      message.slaveCaptureTime = firstSlaveCaptureTime;//slave start time
      // delay(2000);
      radio.sendMessage(message);

      while(message.count<pktCount){
        transmitLoop();
      }

      messageCount=0;
      message.count=0;
      m=0;
      sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0,sumY2=0;
      radio.startListening();
    }

    
};
#endif // SLAVEDEVICE_H





