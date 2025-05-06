#include "config.h"

int pktCount = 140; // Number of samples to collect before switching
int messageCount = 0;

int runNum=0;
bool skipNext=false;

struct MasterData {
  //_s is info saved by slave when listening
    unsigned long long masterTimeStart_s;
    unsigned long long masterTimeEnd_s;
    unsigned long long slaveTimeStart_s;
    unsigned long long slaveTimeEnd_s;
  //_m is info saved by slave as transmitter+ first pkt from master
    unsigned long long masterTimeStart_m;
    unsigned long long masterTimeEnd_m;
    unsigned long long slaveTimeStart_m;
    unsigned long long slaveTimeEnd_m;
    double alpha_master;
    double beta_master;
    double alpha_slave;
    double beta_slave;
};

MasterData master1;
MasterData master2;

bool isFirstTime=true;
bool computed=false;//ensure only one calculation for first 5 pkts

struct Previous {
    unsigned long long masterCaptureTime;
    unsigned long long slaveCaptureTime;
    int messageCount;
    double alpha;
    double beta;
} previous;

// Use long long int for sums to try and prevent overflow and increase precision
unsigned long long int sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0,sumY2=0;
volatile unsigned long long firstMasterCaptureTime, firstSlaveCaptureTime, endMasterCaptureTime;

double alpha = 0.0;
double alpha_s = 0.0;
double beta = 0.0;
int n = 0; // Message count
int m = 0; // Skip counter
Message message;
unsigned long long masterTime = 0.0;
unsigned long long slaveTime = 0.0;

int previousRaceCount=0;

void receiveLoop() {
    bool tx_ok, tx_fail, rx_ready;
    
    if (radio.interruptFlag) {
        radio.interruptFlag = false;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
            radio.flushBuffer();
            snprintf(message.messageType, sizeof(message.messageType), "NULL");
        }

        if (tx_fail) {
            Serial.println("Slave Failed!");
        }

        if (rx_ready) {
            armed = true;
            message = radio.receiveMessage();
            radio.flushBuffer();

            if (message.count==0) { // Second last message, store alpha and beta
                //calculate master Alpha=
                // double alpha=alpha = message.alpha + (double)(message.slaveCaptureTime-master1.masterTimeStart_s) - beta * (double)(master1.slaveTimeStart_m-master1.masterTimeStart_s);
                master1.alpha_master = message.alpha;
                master1.beta_master = message.beta;
                master1.masterTimeStart_m = message.slaveCaptureTime;
                master1.masterTimeEnd_m = message.masterCaptureTime;
                master1.alpha_slave=previous.alpha;
                master1.beta_slave=previous.beta;
                // Serial.print("First Master Capture Time: ");
                // Serial.println((unsigned long)master1.masterTimeStart);
                // Print debug information for previous packets
                  
                  //previous  pkts data
                    Serial.print("Alpha = ");
                    Serial.println(master1.alpha_slave, 6);//alpha_s_s
                    Serial.print("Beta = ");
                    Serial.println(master1.beta_slave, 6);
                    Serial.print("Alpha2 = ");
                    Serial.println(master1.alpha_master, 6); //alpha_s_m
                    Serial.print("Beta2 = ");
                    Serial.println(master1.beta_master, 6);
                    Serial.print("FirstSlaveCaptureTime_S: ");
                    radio.printLongLong(master1.slaveTimeStart_s);
                    Serial.println();
                    Serial.print("EndSlaveCaptureTime_S: ");
                    radio.printLongLong(master1.slaveTimeEnd_s);
                    Serial.println();
                    Serial.print("FirstMasterCaptureTime_S: ");
                    radio.printLongLong(master1.masterTimeStart_s);
                    Serial.println();
                    Serial.print("EndMasterCaptureTime_S: ");
                    radio.printLongLong(master1.masterTimeEnd_s);
                    Serial.println();
                    Serial.print("FirstSlaveCaptureTime_M: ");
                    radio.printLongLong(master1.slaveTimeStart_m);
                    Serial.println();
                    Serial.print("EndSlaveCaptureTime_M: ");
                    radio.printLongLong(master1.slaveTimeEnd_m);
                    Serial.println();
                    Serial.print("FirstMasterCaptureTime_M: ");
                    radio.printLongLong(master1.masterTimeStart_m);
                    Serial.println();
                    Serial.print("EndMasterCaptureTime_M: ");
                    radio.printLongLong(master1.masterTimeEnd_m);
                    
                    n++;
                    return;
                }
                
            }

            // Save capture time
            noInterrupts();
            message.slaveCaptureTime = totalCaptureTime;
            interrupts();
            // Serial.print("Total cap time: ");
            // Serial.println((unsigned long)totalCaptureTime);
            // Serial.print("Slave cap time: ");
            // Serial.println((unsigned long)message.slaveCaptureTime);

            // if(message.count<5){
            //   return; //do nothing
            // }

            if (n==1) {
                // Save the first master and slave capture times as offsets
                runNum++;
                isFirstTime=false;
                computed=false;
                previous.slaveCaptureTime = message.slaveCaptureTime;
                previous.masterCaptureTime = message.masterCaptureTime;
                previous.messageCount = message.count;
                n++;
                Serial.println("****");
                return;
            }



            if (n == 2) {
                firstMasterCaptureTime = message.masterCaptureTime;
                firstSlaveCaptureTime = previous.slaveCaptureTime;
                previous.slaveCaptureTime = message.slaveCaptureTime;
                previous.masterCaptureTime = message.masterCaptureTime;
                previous.messageCount=message.count;             

                Serial.println((unsigned long)firstSlaveCaptureTime);
                Serial.print("First Master Capture Time: ");
                Serial.println((unsigned long)firstMasterCaptureTime);

                if(strcmp(radio.radioID,"Slav1")==0){
                      //Tend_m+T2_m
                      unsigned long long t1_avg=(master1.masterTimeEnd_m/2+firstMasterCaptureTime/2);
                      //Beta_s(x4-T1_m)
                      double y_0=radio.computeDiff(master1.beta_slave,t1_avg,master1.masterTimeEnd_m);
                      //master2.beta_slave*(double)(t1_avg-master1.masterTimeStart_m);
                      //Beta_m(x4-T2_m)
                      double y_1=radio.computeDiff(master1.beta_master,t1_avg,firstMasterCaptureTime);
                      //master1.beta_master*(double)(t1_avg-firstMasterCaptureTime);
                      //alpha_s_s-alpha_s_m
                      double y_3=(master1.alpha_master-master1.alpha_slave);
                      double equation2=y_1-abs(y_0)+y_3;

                       //reversed from the python
                      // unsigned long long t1_avg=(master1.masterTimeEnd_m/2+firstMasterCaptureTime/2);
                      Serial.print("xmid=");
                      radio.printLongLong(master1.masterTimeEnd_m);
                      Serial.print(" + ");
                      radio.printLongLong(firstMasterCaptureTime);
                      Serial.print("   =>  ");
                      radio.printLongLong(t1_avg);
                      Serial.println();

                      Serial.print(" y1 ");
                      radio.printLongLong(y_1);
                      Serial.print(" y2 ");
                      Serial.println(abs(y_0));
                      Serial.print(" y1 ");
                      Serial.println(y_3);
                      //double* long long+ double 

                      // double y1=radio.computeDiff(master1.beta_slave,t1_avg,master1.alpha_slave);
                      // //(master2.beta_slave*1e3*t1_avg)/1e6+master1.alpha_slave;

                      
                      // double y2=radio.computeDiff(master1.beta_master,t1_avg,master1.alpha_master);
                      // //(master1.beta_master*1e3*t1_avg)/1e6+master1.alpha_master;
                      // double equation1=(double)(y1-y2);

                      // Serial.print("Y[0]=");
                      // Serial.print(master1.beta_slave,6);
                      // Serial.print(" * ");
                      // radio.printLongLong(t1_avg);
                      // Serial.print(" + ");
                      // Serial.print(master1.alpha_slave);
                      // Serial.print("  =>  ");
                      // Serial.print(y1);

                      // Serial.println();

                      // Serial.print("Y[1]=");
                      // Serial.print(master1.beta_master,6);
                      // Serial.print(" * ");
                      // radio.printLongLong(t1_avg);
                      // Serial.print(" + ");
                      // Serial.print(master1.alpha_master);
                      // Serial.print("  =>  ");
                      // Serial.print(y2);
                      //  Serial.println();

                      // Serial.print("Res=");
                      // Serial.println((double)equation1);    
                      Serial.print("Res2=");
                      Serial.println((double)equation2);                   
                }

                if(strcmp(radio.radioID,"Slav1")==0){
                      //slave output
                      Serial.print("ts=");
                      Serial.print(message.count);
                      Serial.print(" ");
                      radio.printLongLong(firstMasterCaptureTime);
                      Serial.print(" ");
                      radio.printLongLong(firstSlaveCaptureTime);
                      Serial.print(" ");
                      Serial.print(runNum);
                      Serial.println();               
                      
                    }else{
                      Serial.print("tm=");
                      Serial.print(message.count);
                      Serial.print(" ");
                      radio.printLongLong(firstSlaveCaptureTime);
                      Serial.print(F(" "));
                      radio.printLongLong(firstMasterCaptureTime);
                      Serial.print(" ");
                      Serial.print(runNum);
                      Serial.println();
                    }

                n++;
                return;
            }

            if(skipNext){
              skipNext=false;
              previous.slaveCaptureTime=message.slaveCaptureTime;
              previous.messageCount=message.count;
              return;
            }

            if((message.slaveCaptureTime-previous.slaveCaptureTime)>1.32e6 && (message.slaveCaptureTime-previous.slaveCaptureTime)<3.2e6){
              //skip packet
              skipNext=true;
              previousRaceCount=raceCount;
              Serial.println("RaceCount!");
            }



            if (previous.messageCount + 1 == message.count) {
              // if (true) {
                previous.messageCount=message.count;
                masterTime = message.masterCaptureTime;
                slaveTime = previous.slaveCaptureTime;
                previous.masterCaptureTime = message.masterCaptureTime;
                previous.slaveCaptureTime = message.slaveCaptureTime;

                // Serial.print("MasterTime: ");
                // Serial.println((unsigned long)masterTime, 6);
                // Serial.print("Slave: ");
                // Serial.println((unsigned long)slaveTime, 6);

              
                if (message.count>0 ) {//no operations on first 0 pkts
                    // Apply offset by subtracting the first master and slave capture times
                    uint64_t normalizedX = masterTime - firstMasterCaptureTime;
                    uint64_t normalizedY = slaveTime - firstSlaveCaptureTime;

                    // Accumulate the values for regression
                    sumX += normalizedX;
                    sumY += normalizedY;
                    sumXY += normalizedX * normalizedY;
                    sumX2 += normalizedX * normalizedX;
                    sumY2 += normalizedY * normalizedY;
                    m++;
                    
                    // Serial.print("m: ");
                    // Serial.println(m);
                    // Serial.print("NormalisedX: ");
                    // Serial.println(normalizedX, 6);
                    // Serial.print("NormalizedY: ");
                    // Serial.println(normalizedY, 6);

                    if(strcmp(radio.radioID,"Slav1")==0){
                      //slave output
                      Serial.print("ts=");
                      Serial.print(message.count);
                      Serial.print(" ");
                      radio.printLongLong(masterTime);
                      Serial.print(" ");
                      radio.printLongLong(slaveTime);
                      Serial.print(" ");
                      Serial.print(runNum);
                      Serial.println();               
                      
                    }else{
                      Serial.print("tm=");
                      Serial.print(message.count);
                      Serial.print(" ");
                      radio.printLongLong(slaveTime);//master time since master is slave
                      Serial.print(" ");
                      radio.printLongLong(masterTime);
                      Serial.print(" ");
                      Serial.print(runNum);
                      Serial.println();
                    }

                }
                n++;

                // Debugging output for every 100th message
                if (n % 100 == 0) {
                    Serial.print("OVF: ");
                    Serial.println(OVF);
                    Serial.println();
                }
            } else {
                Serial.println("Skip");
                Serial.println(message.count);
                Serial.println(previous.messageCount);
                previous.messageCount=message.count;
                
            }

            if (message.count == pktCount) {
                // Calculate the linear regression
                Serial.println("**************");
                double denominator=0.0;

                if(strcmp(radio.radioID,"Slav1")==0){ //if slave               
                  denominator =radio.computeDenominator(m,sumX2,sumX);

                  // Prevent division by zero if the denominator is too small
                  if (fabs(denominator) < 1e-9) {
                    Serial.println("Warning: Denominator is too small. Regression cannot be computed.");
                  } else {
                    // Calculate alpha (intercept) and beta (slope)
                    beta = radio.computeBetaNum(m,sumXY,sumX,sumY) / denominator;
                    //double computeAlphaNum(unsigned long long sumY,unsigned long long sumX2,unsigned long long sumX,unsigned long long sumXY){
                    alpha_s = radio.computeAlphaNum(sumY,sumX2,sumX,sumXY)/ denominator;

                    // Adjust alpha and beta with the offsets
                    //alpha = alpha_s + (double)firstSlaveCaptureTime - beta * (double)firstMasterCaptureTime;
                    // alpha = alpha_s + (double)(firstSlaveCaptureTime-master1.slaveTimeStart_s) - beta * (double)(firstMasterCaptureTime-master1.masterTimeStart_m);

                  }
                }
                else{
                  
                  denominator = radio.computeDenominator(m,sumY2,sumY);
            
                  // Prevent division by zero if the denominator is too small
                  if (fabs(denominator) < 1e-9) {
                    Serial.println("Warning: Denominator is too small. Regression cannot be computed.");
                    } else {
                     // Calculate alpha (intercept) and beta (slope)
                     beta = radio.computeBetaNum(m,sumXY,sumY,sumX)  / denominator;
                     alpha_s = radio.computeAlphaNum(sumX,sumY2,sumY,sumXY)/ denominator;

                    // Adjust alpha and beta with the offsets
                    //alpha = alpha_s + (double)firstMasterCaptureTime - beta * (double)firstSlaveCaptureTime;
                    // alpha = alpha_s + (double)(firstMasterCaptureTime-master1.masterTimeStart_m) - beta * (double)(firstSlaveCaptureTime-master1.masterTimeStart_m);
                  }
                }

                    previous.alpha=alpha_s;
                    previous.beta=beta;
                    master1.masterTimeStart_s=firstMasterCaptureTime;
                    master1.masterTimeEnd_s=message.masterCaptureTime;
                    master1.slaveTimeStart_s=firstSlaveCaptureTime;
                    master1.slaveTimeEnd_s=message.slaveCaptureTime;

                    previous.messageCount=0;

                    
                    Serial.print("SumX2=");
                    radio.printLongLong(sumX2);
                    Serial.println();
                    Serial.print("SumXY=");
                    radio.printLongLong(sumXY);
                    Serial.println();
                    Serial.print("SumX=");
                    radio.printLongLong(sumX);
                    Serial.println();
                    Serial.print("SumY= ");
                    radio.printLongLong(sumY);
                    Serial.println();
                    Serial.print("SumY2= ");
                    radio.printLongLong(sumY2);
                    Serial.println();
                    Serial.print("m=");
                    Serial.println(m);
                    Serial.print("Denominator: ");
                    Serial.println(denominator/1e15,16);
                    Serial.print("beta: ");
                    Serial.println(beta, 6);
                    Serial.print("alpha: ");
                    Serial.println(alpha, 6);
                if(strcmp(radio.radioID,"Slav1")==0){
                  //slave output
                  Serial.print("fs=");
                  Serial.print(alpha,10);
                  Serial.print(" ");
                  Serial.print(beta,10);
                  Serial.print(" ");//tbm
                  radio.printLongLong(firstMasterCaptureTime);
                  Serial.print(" ");//tem
                  radio.printLongLong(masterTime);
                  Serial.print(" ");//tbs
                  radio.printLongLong(firstSlaveCaptureTime);
                  Serial.print(" ");//tes                  
                  radio.printLongLong(slaveTime);
                  Serial.print(" ");
                  Serial.print(alpha_s,10);
                  Serial.print(" ");
                  Serial.print(runNum);
                  Serial.print(" ");
                  Serial.print(raceCount);
                  Serial.println();

                  // fs=alpha_s beta_s tbm_s tem_s tbs_s tes_s
                  //fm=alpha_m beta_m tbm_m tem_m tbs_m tes_m

                }
                else{
                  Serial.print("fm=");
                  Serial.print(alpha,10);
                  Serial.print(" ");
                  Serial.print(beta,10);
                  Serial.print(" ");//tbm
                  radio.printLongLong(firstSlaveCaptureTime);
                  Serial.print(" ");//tem
                  radio.printLongLong(slaveTime);
                  Serial.print(" ");//tbs
                  radio.printLongLong(firstMasterCaptureTime);
                  Serial.print(" ");//tes
                  radio.printLongLong(masterTime);
                  Serial.print(" ");
                  Serial.print(alpha_s,10);
                  Serial.print(" ");
                  Serial.print(runNum);
                  Serial.print(" ");
                  Serial.print(raceCount);
                  Serial.println();
                }

            }
            
            previous.messageCount = message.count;
            // previous.masterCaptureTime = message.masterCaptureTime;
            // previous.slaveCaptureTime = message.slaveCaptureTime;
            
        
    }
}

void transmitLoop() {
    bool tx_ok, tx_fail, rx_ready;
    
    if (radio.instance->interruptFlag) {
        radio.interruptFlag = false;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
            radio.flushBuffer();
            armed = true;
            OVF = overFlowCount;
            radio.startListening();
        } else if (tx_fail) {
            Serial.println("Master Failed!");
        }
    }

    if (overFlowCount - OVF > 5) { // Send a message after 10 overflow
        snprintf(message.messageType, sizeof(message.messageType), "DATA");

        noInterrupts();
        message.masterCaptureTime = totalCaptureTime;
        interrupts();

        messageCount++;
        message.count = messageCount;
        
        if(messageCount==1 && strcmp(radio.radioID,"Slav1")==0){
          //slave recorded data
          master1.slaveTimeStart_m=message.masterCaptureTime;
        }

        if (message.count==1 && strcmp(radio.radioID,"Slav1")!=0){
          //master also records its own capture time
          firstMasterCaptureTime=message.masterCaptureTime; 
          master1.masterTimeStart_m=message.masterCaptureTime;
          Serial.println((unsigned long)firstMasterCaptureTime); 
        }


        if(messageCount==pktCount && strcmp(radio.radioID,"Slav1")==0){
          //slave recorded data
          master1.slaveTimeEnd_m=message.masterCaptureTime;

        }

        if(message.count==pktCount && strcmp(radio.radioID,"Slav1")!=0){
          //Master end time
          endMasterCaptureTime=message.masterCaptureTime;
          master1.masterTimeEnd_m=message.masterCaptureTime;
          Serial.println((unsigned long)endMasterCaptureTime);
        }

        // if (message.count <5) {//send this information in the last 5 pkts
            // message.masterCaptureTime = previous.slaveCaptureTime;//masterEndTime
            message.slaveCaptureTime = firstSlaveCaptureTime;//master start time
        //     message.beta = previous. beta;
        //     message.alpha = previous.alpha;
        //     radio.sendMessage(message);

        // } else {
            // message.slaveCaptureTime = 0;
            radio.sendMessage(message);
        // }
        
        if (messageCount % 100 == 0) {
            Serial.print(" . ");
        }
    }
}

void superMasterLoop() {
    bool tx_ok, tx_fail, rx_ready;
    
    if (radio.instance->interruptFlag) {
        radio.interruptFlag = false;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
            radio.flushBuffer();
            armed = true;
            OVF = overFlowCount;
        } else if (tx_fail) {
            Serial.println("Master Failed!");
        }
    }

    if (overFlowCount - OVF > 10) { // Send a message after 10 overflow
    
        newMessage.messageType='A';
        newMessage.stationID='0';

        noInterrupts();
        newMessage.captureTime=totalCaptureTime*256;
        interrupts();

        messageCount++;
        newMessage.seqNum=messageCount;
        
        Serial.print("SeqNum: ");
        Serial.print(newMessage.seqNum);
        Serial.print(" stationID: ");
        Serial.print(newMessage.stationID);
        Serial.print(" message_t: ");
        Serial.print(newMessage.messageType);
        Serial.print(" captureTime: ");
        unsigned long lowerPart = newMessage.captureTime & 0xFFFFFFFF;  // lower 4 bytes
        unsigned long upperPart = (newMessage.captureTime >> 32) & 0xFFFFFFFF;  // upper 4bytes
        Serial.print((unsigned long)lowerPart);
        Serial.print(" ");
        Serial.print((unsigned long)upperPart);

        Serial.println();
        
        if (messageCount % 100 == 0) {
            Serial.print(" . ");
        }
    }

}
