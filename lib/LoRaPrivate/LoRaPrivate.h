#ifndef LoRaPrivate_H
#define LoRaPrivate_H

#include <platformTypes.h>
#include <LoRaCfg.h>

uint8 LoRaConfig(void);
bool isChannelBusy(void);
bool isDataDuplicated(void);
uint8 sendPacket(uint8* packet, uint16 packet_len);
//uint8 awaitAck(void);
uint8 replyAck(void);
uint8 replyCalendarTime(void);
void onCadDone(bool signalDetected);
//void onTxDone(void);
void onReceive(int packetSize);

volatile extern uint8 in_packet[IN_BUFFER_SIZE];
volatile extern uint16 in_packet_len;

#endif //LoRaPrivate_H
