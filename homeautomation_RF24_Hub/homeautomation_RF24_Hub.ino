#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
#include "printf.h"


/*
This is the first version of my attempt at the home automation hub
*/


RF24 radio(9,10);
char receivePayload[32];
uint8_t len = 0;
uint8_t pipe = 0;


// All pipes to be used by sensors here 
// P0 - Hub Pipe for writing. Read by all sensors
// P1-P5 - Sensor pipes for writing by sensors. Hub reads on these. 

//const uint64_t pipes[6] = { 0x7365727631LL, 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL, 0xF0F0F0F0E5LL };
const uint64_t pipes[6] = { 0x7365727631LL, 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL, 0xF0F0F0F0E5LL };

// P1: Garage Hall Effect Slave 

void setup(void)
{
  Serial.begin(57600);
  printf_begin();

  radio.begin();
  
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(76);
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  
  radio.setCRCLength(RF24_CRC_16);
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  radio.startListening();
  radio.printDetails();
  
  delay(1000);

}

void loop (void) 
{
  
  if (radio.available(&pipe))
  {
    bool done = false; 
    while (!done)
    {
      len = radio.getDynamicPayloadSize();
      done = radio.read( &receivePayload, len);
      
      radio.stopListening();
      //radio.write(receivePayload,len);
      
      receivePayload[len] = 0;
      printf("Got payload: %s len:%i pipe:%i\n\r",receivePayload,len,pipe);

      switch(pipe)
      {
        case 0:
          hubPipeHandler(receivePayload);
          break;
        case 1:
          garageDoorSensorHandler(receivePayload);
          break;
        default:
          defaultHandler(receivePayload);
          break;
      }
      
      radio.startListening();
      pipe++;
      
      if(pipe > 5) pipe = 0;
      
    }
  }
  
  delay(20);
}


void garageDoorSensorHandler(char receivePayload[])
{
  printf("Garage Door Activated");
}


void hubPipeHandler(char receivePayload[])
{}

void defaultHandler(char receivePayload[])
{}

