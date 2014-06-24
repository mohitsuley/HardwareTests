#include <SPI.h>
#include <RF24.h>
#include "printf.h"

RF24 radio(9,10);

// For best performance, use P1-P5 for writing and Pipe0 for reading as per the hub setting
// Below is the settings from the hub/receiver listening to P0 to P5
//const uint64_t pipes[6] = { 0x7365727631LL, 0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL, 0xF0F0F0F0E5LL };

// Pipes are unique per slave unit. 
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0x7365727631LL };

char receivePayload[32];
uint8_t counter=0;


void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\r Hall Effect Slave");
  //printf("ROLE: %s\n\r",role_friendly_name[role]);

  radio.begin();
  radio.enableDynamicPayloads() ;

  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(76);
  radio.setRetries(15,15);

  radio.openWritingPipe(pipes[0]); 
  radio.openReadingPipe(1,pipes[1]); 
  
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
  
  sprintf(txBuffer,"%2X", nodeID);
  strcat(txBuffer,",");
  
  // First byte of message, just a counter. Will update others later on. 
  Data1 = counter++;
  sprintf(temp,"%03d",Data1);
  strcat(txBuffer,temp);
  


  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %l",time);
    bool ok = radio.write( &time, sizeof(unsigned long) );
    
    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 1+(radio.getMaxTimeout()/1000) )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
      printf("Timeout duration: %d\n\r", (1+radio.getMaxTimeout()/1000) ) ;
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &got_time, sizeof(unsigned long) );
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back. This way, we don't delay
      // the reply while we wait on serial i/o.
      radio.write( &got_time, sizeof(unsigned long) );
      printf("Sent response %lu\n\r", got_time);

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
