#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
#include "printf.h"

RF24 radio(9,10);

// All pipes to be used by sensors here 
// P0 - Hub Pipe for writing. Read by all sensors
// P1-P5 - Sensor pipes for writing by sensors. Hub reads on these. 

// P0 is Hub Pipe to be read by sensors. P1 is specific sensor pipe; to be opened for writing. 
const uint64_t pipes[2] = { 0x7365727631LL, 0xF0F0F0F0E1LL };
char receivePayload[32];
uint8_t counter=0;


void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\r Hall Effect Slave");
  
  radio.begin();
  radio.enableDynamicPayloads() ;

  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(76);
  radio.setRetries(15,15);

  radio.openReadingPipe(0,pipes[0]); 
  radio.openWritingPipe(pipes[1]); 
  
  radio.setAutoAck( true ) ;
  radio.printDetails();
  delay(1000);
}

void loop(void)
{
  // Get last two digits of the node-ID
  uint16_t nodeID = pipes[0] & 0xff;
  uint8_t Data1, Data2, Data3, Data4 = 0;
  char txBuffer[32]= "";
  char temp[5];
  usigned long send_time, rtt = 0;
  
  sprintf(txBuffer,"%2X", nodeID);
  strcat(txBuffer,",");
  
  // First byte of message, just a counter. Will update others later on. 
  Data1 = counter++;
  sprintf(temp,"%03d",Data1);
  strcat(txBuffer,temp);
  
  printf("txBuffer: %s len: %d\n\r",txBuffer, strlen(txBuffer));

  radio.stopListening();
  
  if(radio.write(txBuffer, strlen(txBuffer))) 
  {
    printf("Send successful\n\r"); 
  }
  else
  {
    printf("Send failed\n\r");
  }
  
  radio.startListening();
  delay(20);
  
  send_time = millis();
  
  while (radio.available() && !timeout)
  {
    uint8_t len = radio.getDynamicPayloadSize();
    radio.read( receivePayload, len); 
         
    receivePayload[len] = 0;
    printf("rxBuffer:  %s\n\r",receivePayload);
    
    if ( ! strcmp(txBuffer, receivePayload) ) 
    {
      rtt = millis() - send_time;
      printf("inBuffer --> rtt: %i \n\r",rtt);      
    }
    
    if ( millis() - send_time > radio.getMaxTimeout() ) 
    {
         Serial.println("Timeout!!!");
         timeout = 1;
    }

    delay(10);  
  }
  
  delay(250);
}
