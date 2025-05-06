#ifndef NRF24RADIO_H
#define NRF24RADIO_H

#include "config.h"
#include <RF24.h>
#include <printf.h>

volatile bool armed=false;
volatile bool finished=true; //read only outside ISR!!!!
volatile unsigned long overFlowCount; //read only outside ISR!!!!
volatile unsigned long midPointCount; //read only outside ISR!!!!
volatile unsigned long startOverFlowCount; //read only outside ISR!!!!
volatile unsigned int captureTime1; //read only outside ISR!!!!
volatile unsigned int captureTime2; //read only outside ISR!!!!
volatile unsigned long long totalCaptureTime=0;
volatile unsigned long OVF = 0;
int currentChannel=78;
volatile int raceCount=0;
unsigned int clkval=0;

class NRF24Radio {
public:
    RF24 radio;
    volatile bool interruptFlag = false;       // Interrupt flag for nRF24 IRQ
    static NRF24Radio* instance;              // Static pointer to the instance
    char radioID[6];//stores radio identity, ie Mast1, Mast2,or Slave1
    
    //timeout flag
    volatile bool timeOutFlag;

    // Variables for nRF24 IRQ handling
    volatile bool tx_ok = false, tx_fail = false, rx_ready = false;

    NRF24Radio(byte cePin, byte csnPin)
        : radio(cePin, csnPin,1000000) {}

    void initialize(const byte* address, const byte* addressAck) {      
        while(!radio.begin())//wait till radio is initialised

        radio.setDataRate(RF24_1MBPS);         // Set data rate
        radio.setPALevel(RF24_PA_LOW);         // Set PA level
        radio.setAutoAck(false);
        radio.setPayloadSize(sizeof(Message)); // Set payload size
        radio.openWritingPipe(addressAck);        // Open writing pipe
        radio.openReadingPipe(0, address);  // Open reading pipe
        radio.startListening();                // Start listening
        radio.maskIRQ(0, 1, 0);                // Mask TX_DS and MAX_RT interrupts, enable RX_DR

        pinMode(IRQ_PIN, INPUT_PULLUP);
        // pinMode(PUSH_BUTTON, INPUT_PULLUP);
        pinMode(CAPTURE_DEBUG_PIN, OUTPUT);
        pinMode(DEBUG_PIN, OUTPUT);
        pinMode(OVF_CLOCK_PIN,OUTPUT);
        pinMode(DEBUG_CLOCK_PIN,OUTPUT);

        instance = this; // Set static instance pointer
        attachInterrupt(digitalPinToInterrupt(IRQ_PIN), interruptRoutine, FALLING);
        setupInputCapture(); // Configure timer for input capture
    
    }

    void setChannel(int channel){
        radio.setChannel(channel);
    }

    void hopChannel(){
        //channel hopper
    switch(currentChannel){
      case 78:
        radio.setChannel(90);
        Serial.println("Set channel to 90!");
        currentChannel=90;
        Serial.println(radio.getChannel());      
        break;
      case 90:
      //     radio.setChannel(100);
      //     Serial.println(radio.getChannel());
      //     currentChannel=100;
      //     Serial.println("Set channel to 100!");
      //     break;
      // case 100:
          radio.setChannel(78);
          currentChannel=78;
          Serial.println("Set channel to 78!");
          break;
      default:
        Serial.println("Lost in changing Channels!");
    }
  }

    void printDetails() {
        printf_begin();
        radio.printPrettyDetails();
    }

    void startListening(){
      radio.startListening();
    }

    void stopListening(){
      radio.stopListening();
    }

  void flushBuffer(){
    radio.flush_rx();
    radio.flush_tx();
  }
  
  Message receiveMessage(){
    Message dataReceived;
    if(radio.available()){
      radio.read(&dataReceived, sizeof(dataReceived));

      #if DEBUG_MODE_1
        
        Serial.print(F("{Received Message Type: "));
        Serial.print(dataReceived.messageType);
        Serial.print(F(", Count: "));
        Serial.print(dataReceived.count);
        Serial.print(F(", master Capture Time: "));
        Serial.print((unsigned long)dataReceived.masterCaptureTime);
        Serial.print(F(", Slave Capture Time: "));
        Serial.print((unsigned long)dataReceived.slaveCaptureTime);
        Serial.println(F("}"));
      #else
        delay(60);
      #endif
        
        return dataReceived;
      }

      //not available; return NULL, never happens
      sprintf(dataReceived.messageType,sizeof(dataReceived.messageType),"NULL");
      return dataReceived;
    }

    // message: count, captureTime
    // messagetype is handled by handleprotocol()
    void sendMessage(Message message){
      radio.stopListening();
      Message dataToSend=message;
      dataToSend.slaveCaptureTime=message.slaveCaptureTime;
      dataToSend.masterCaptureTime=message.masterCaptureTime;
      dataToSend.count=message.count;
      radio.startFastWrite(&dataToSend, sizeof(dataToSend), 0);

      #if DEBUG_MODE_1

      Serial.print(F("{Sent Message Type: "));
      Serial.print(dataToSend.messageType);
      Serial.print(F(", Count: "));
      Serial.print(dataToSend.count);
      Serial.print(F(", master Capture Time: "));
      printLongLong(dataToSend.masterCaptureTime);
      Serial.print(F(", Slave Capture Time: "));
      printLongLong(dataToSend.slaveCaptureTime);
      Serial.println(F("}"));
      #else{
      delay(60);
      #endif
    }

    void printLongLong(unsigned long long num){
      Serial.print((unsigned long)(num/(1llu<<32)));
      Serial.print(" ");
      Serial.print((unsigned long)(num%(1llu<<32)));
    }

    double ll2f(unsigned long long num){//unsigned long long to double
      unsigned long long high=num/(1llu<<32);
      unsigned long long low=num%(1llu<<32);
      
      double result= (double)high*4294967296.0+(double) low;

      return result;
    }

    double computeDenominator(int m,unsigned long long sumX2,unsigned long long sumX){
      unsigned long long a,b,c,d;
      a=sumX2/4294967296;
      b=sumX2%4294967296;
      c=sumX/4294967296;
      d=sumX%4294967296;
      double res=(double)( m*a -((c*c)<<32)-(c*d)-(c*d));
      double mbd=(double) (m*b-d*d);
      double mb=(double) (m*b);
      double dd=(double) (d*d);

      if (m*b>=d*d){
        return res*4294967296.0+(double) (m*b-d*d);
      }
      else{
        return res*4294967296.0-((double)(d*d-m*b));
      }
    }
    double computeBetaNum(int m,unsigned long long sumXY,unsigned long long sumX,unsigned long long sumY){
      unsigned long long a,b,c,d,e,f;
      a=sumXY/4294967296;
      b=sumXY%4294967296;
      c=sumX/4294967296;
      d=sumX%4294967296;
      e=sumY/4294967296;
      f=sumY%4294967296;

      double res=(double)( m*a -((c*e)<<32)-(c*f)-(d*e));
      
      if (m*b>=d*f){
        return res*4294967296.0+(double) (m*b-d*f);
      }
      else{
        return res*4294967296.0-((double)(d*f-m*b));
      }
     
    }
    //((double)sumY * (double)sumX2 - (double)sumX * (double)sumXY)
    double computeAlphaNum(unsigned long long sumY,unsigned long long sumX2,unsigned long long sumX,unsigned long long sumXY){
      unsigned long long a,b,c,d,e,f,g,h;
      a=sumY/4294967296;
      b=sumY%4294967296;
      c=sumX2/4294967296;
      d=sumX2%4294967296;
      e=sumX/4294967296;
      f=sumX%4294967296;
      g=sumXY/4294967296;
      h=sumXY%4294967296;

      double res;
      if (a*c>=e*g){
        res = (double)(a*c-e*g);
      }
      else{
        res = -(double)(e*g-a*c);
      }
      double res2;
      if(a*d>=e*h){
        res2=(double)(a*d-e*h);
      }
      else{
        res2=-(double)(e*h-a*d);
      }
      if(b*c>=f*g){
        res2=res2+(double)(b*c-f*g);
      }
      else{
        res2=res2-(double)(f*g-b*c);
      }
      if(b*d>=f*h){
        res2=res2*4294967296.0+(double)(b*d-f*h);
      }
      else{
        res2=res2*4294967296.0-(double)(f*h-b*d);
      }

      return res*4294967296.0*4294967296.0+res2;
     
    }

    double computeDiff(double beta, unsigned long long t1_avg,unsigned long long tm ){
      unsigned long long t1_h,tm_h,t1_l,tm_l;
      t1_h=t1_avg/4294967296;
      t1_l=t1_avg%4294967296;
      tm_h=tm/4294967296;
      tm_l=tm%4294967296;

      double res_h=0.0;
      if(t1_h>tm_h){
        res_h=(double)((t1_h-tm_h)*beta)*4294967296;
      }
      else{
        res_h=(double)((tm_h-t1_h)*beta)*4294967296;
      }
      
      double res_l=0.0;
      if(t1_l>tm_l){
        res_l=(double)((t1_l-tm_l)*beta);
      }
      else{
        res_l=(double)((tm_l-t1_l)*beta);
      }
      

      return res_h+res_l;
    }

    static void interruptRoutine() {
      instance->interruptFlag = true; // Set the instance-specific interrupt flag
    }

    void setupInputCapture() {
        noInterrupts(); // Disable interrupts during setup

        // Configure Timer1 for input capture, normal mode, no prescaler
        TCCR1A = 0;
        TCCR1B = (1 << CS10); // No prescaler, input capture on falling edge

        // Enable input capture interrupt and overflow interrupt
        TIMSK1 |= (1 << ICIE1); // Input capture interrupt
        TIMSK1 |= (1 << TOIE1); // Overflow interrupt
        
        TIMSK1 |= (1 <<  OCIE1A); //Enable output compare interrupt  OCIEB OCIE1A
        OCR1A = 32768;

        overFlowCount= 0;
        midPointCount= 0;
        TCNT1=5; // reseting the timer to get a good initial value(midpoint comes first)
        TIFR1 = 0;
        interrupts(); // Enable interrupts
    }
  
    // Handle Timeout
    void timeOut() {
      int timeoutCounter = 0;
      radio.startListening();
      while (!radio.available() && timeoutCounter < 150) {
          delay(10);
          timeoutCounter++;
          if (timeoutCounter >= 150) {
              instance->timeOutFlag = true;
              instance->flushBuffer();
              Serial.println(F("Time out"));
              break;
      }
    }
    
}};

   // ISR for Timer1 Input Capture
   ISR(TIMER1_CAPT_vect,ISR_NOBLOCK) {
        // if (!armed && !finished)
        //     {
        //       captureTime2 = ICR1;
        //       debugPin(false);
        //       startOverFlowCount = overFlowCount - startOverFlowCount;
        //       finished=true;
        //       if (captureTime2<captureTime1) {startOverFlowCount--;}
        //       OVF=startOverFlowCount;
        //       totalCaptureTime =((unsigned long)startOverFlowCount)<<16;
        //       totalCaptureTime += captureTime2 - captureTime1;
        //        absoluteTime = (((unsigned long)overFlowCount)<<16) + captureTime2;
        //     }
       
        if (armed)
            {
              byte ovfflag=TIFR1 &(1<<TOV1);
              clkval=TCNT1;
              captureTime1 = ICR1;
              debugPin(true);
              if(captureTime1 < clkval || ovfflag){
              // if(overFlowCount == midPointCount || captureTime1 > 32768){
                OVF=overFlowCount-startOverFlowCount;
                startOverFlowCount = overFlowCount;
                totalCaptureTime=captureTime1+(((unsigned long long)overFlowCount)<<16);
              }
              else{
                OVF=(overFlowCount-1)-startOverFlowCount;
                startOverFlowCount = overFlowCount-1;
                totalCaptureTime=captureTime1+(((unsigned long long)(overFlowCount-1))<<16);
                raceCount++;
              }
             
              armed=false;
              finished=false;
              captureDebugPin( ((overFlowCount)<<16) + captureTime1);
              // captureDebugPin(0xABCDEF78);



            }
       
 
        }

    
// Define the static instance pointer
NRF24Radio* NRF24Radio::instance = nullptr;

// ISR for Timer1 Overflow (optional, if needed)
ISR(TIMER1_OVF_vect) {
    overFlowCount++;
    ovfClockPin();
    
}

 ISR(TIMER1_COMPA_vect) {
     midPointCount++;
    ovfClockPin();
    
}


#endif // NRF24RADIO_H