#include <Arduino.h>

#include <LoRaPrivate.h>

#include <SPI.h>
#include <LoRa.h>
#include <customUtilities.h>

volatile uint8 in_packet[IN_BUFFER_SIZE];

volatile uint16 in_packet_len = 0;
volatile bool Cad_isr_responded = false;
volatile bool channel_busy = true;
volatile bool ack_received = false;

RTC_DATA_ATTR uint8 previous_did = 0xFF;

//Encapsules the whole LoRa configuration. Returns 0 if successful, 1 if error.
uint8 LoRaConfig(void)
{
  Serial.println("Configuring LoRa");
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(FREQ)) {
    Serial.println("Starting LoRa failed!");
    return 1;
  }
  
  LoRa.setSpreadingFactor(SPR_FACT);
  LoRa.setSignalBandwidth(BANDWIDTH);
  //LoRa.setTxPower(TX_POWER);
  //LoRa.setGain(LNA_GAIN);
  //LoRa.setCodingRate4(CR_DEN);
  //LoRa.setPreambleLength(PREAM_LEN);
  //LoRa.enableInvertIQ();
  //LoRa.setSyncWord(SYNC_WORD);
  LoRa.enableCrc();

  LoRa.onCadDone(onCadDone); //call onCadDone ISR when channel activity detection has finished
  //LoRa.onTxDone(onTxDone); //call onTxDone ISR when packet has been fully sent
  LoRa.onReceive(onReceive); //call onReceive ISR when signals are received
  LoRa.receive();            //gateway set to listen for signals

  return 0;  
}

//Blocking. Checks if someone is using the configured channel. Returns true if used, false if free. 1s timeout
bool isChannelBusy(void)
{
  Cad_isr_responded = false;
  LoRa.channelActivityDetection();
  uint32 start_time = millis();
  while((!Cad_isr_responded) and ((millis() - start_time) < 1000U))
  {
    NOP();
  }
  if(Cad_isr_responded)
  {
    return channel_busy;
  }
  else
  {
    return 1;
  }
}

//ISR called when Channel Activity Detection has finished.
void onCadDone(bool signalDetected) //true means signal is detected
{
  channel_busy = signalDetected;
  Cad_isr_responded = true;
}

//Sends a packet through LoRa. Blocking. Returns 0 if successful, 1 if error
uint8 sendPacket(uint8* packet, uint16 packet_len)
{
  Serial.print("Sending: ");
  printStr(packet, packet_len);
  Serial.println();
  while(!LoRa.beginPacket());
  LoRa.write(packet, packet_len);
  LoRa.endPacket(false); //blocking mode

  return 0;
}

/* NOT USABLE IN GATEWAY
//Waits for acknowledgement for some time. Timeout in ms. Returns 0 if ACK, 1 if no ACK
uint8 awaitAck()
{
  LoRa.receive();
  uint32 start_time = millis();
  while((!ack_received) or ((millis() - start_time) < ACK_TIMEOUT))
  {
    NOP();
  }
  if(ack_received)
  {
    ack_received = false;
    return 0;
  }
  else
  {
    return 1;
  }
}*/

//Sends a small packet with GATEWAY_ID and the EMITTER_ID received through LoRa.
//Blocking: Waits for the channel to be available, with ACK_TIMEOUT.
//Returns 0 if successful, 1 if error.
uint8 replyAck(void)
{
  uint8 err_reg = 1;
  while(!LoRa.beginPacket()); //exit receive mode

  uint32 start_time = millis();
  do
  {
    err_reg = (uint8)isChannelBusy();
  } while(err_reg and ((millis() - start_time) < ACK_TIMEOUT));

  if(!err_reg)
  {
    uint8 out_packet[3] = {(GATEWAY_ID & 0xFF00) >> 8, GATEWAY_ID & 0x00FF, in_packet[GATEWAY_ID_LEN]};
    err_reg = sendPacket(out_packet, sizeof(out_packet));
  }

  LoRa.receive(); //reenter receive mode
  return err_reg;
}

//Checks if the data ID of the newly received packet is different from the last one.
//If this is the case, the data value is new.
//Returns false if data is new, true if data is duplicated.
bool isDataDuplicated(void)
{
  bool returner = true;
  if(in_packet[GATEWAY_ID_LEN + 1U] != previous_did)
  {
    returner = false;
    previous_did = in_packet[GATEWAY_ID_LEN + 1U];
  }

  return returner;
}

//Sends a message containing the gateway ID [0,1], the time message ID[2] and
//the current time in calendar time format (long) split in 4 bytes [3 MSB,6 LSB]
//Timeout of CLDTIME_TIMEOUT. Does not expect ACK.
//Returns 0 if successful, 1 if error
uint8 replyCalendarTime(void)
{
  uint8 err_reg = 1;
  uint8 out_packet[GATEWAY_ID_LEN + 5U];
  out_packet[0] = (GATEWAY_ID & 0xFF00) >> 8;
  out_packet[1] = GATEWAY_ID & 0x00FF;
  out_packet[2] = CLDTIME_MSG_ID; 

  time_t cldtime;

  while(!LoRa.beginPacket()); //exit receive mode

  uint32 start_time = millis();
  do
  {
    time(&cldtime);

    uint8* p_cldtime = (uint8*)&cldtime;
  
    for(uint8 i = GATEWAY_ID_LEN + 1U; i < sizeof(out_packet); i++)
    {
      out_packet[i] = p_cldtime[i- GATEWAY_ID_LEN - 1U]; //little endian arch
    }
      
    err_reg = (uint8)isChannelBusy();
  } while(err_reg and ((millis() - start_time) < CLDTIME_TIMEOUT));

  if(!err_reg)
  {
    err_reg = sendPacket(out_packet, sizeof(out_packet));

    if(!err_reg)
    {
      Serial.print("Sent current calendar time: 0x");
      Serial.print(cldtime, HEX);
      Serial.print(" = DEC ");
      Serial.println(cldtime);
      Serial.print("In string: ");
      printStrHEX(out_packet, sizeof(out_packet));
      Serial.println();
    }
  }

  LoRa.receive(); //reenter receive mode
  return err_reg;
}

/*NOT USED
//ISR called when the sending of data is finished.
void onTxDone(void)
{
  
}*/

//ISR called when detecting LoRa signals in receive mode.
void onReceive(int packetSize)
{
  int w = 0;
  //Checks if the packet is long enough for an ID to fit
  if(packetSize > 1)
  {
    //Reads the ID values and compares to the established ID
    for(w = 0;w<GATEWAY_ID_LEN;w++) in_packet[w] = LoRa.read();
    if(in_packet[0] == (((GATEWAY_ID & 0xFF00) >> 8)) and (in_packet[1] == (GATEWAY_ID & 0x00FF)))
    {
      //Continues if ID is correct
      if(packetSize > IN_BUFFER_SIZE) in_packet_len = IN_BUFFER_SIZE;
      else in_packet_len = packetSize;
      
      for(w = GATEWAY_ID_LEN; w < in_packet_len; w++) in_packet[w] = LoRa.read();
    }
  }
}
