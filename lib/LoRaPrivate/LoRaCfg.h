#ifndef LoRaCfg_H
#define LoRaCfg_H

//SPI pins for interaction with LoRa peripheral unit.
#define SCK         5
#define MISO        19
#define MOSI        27
#define SS          18
#define RST         14
#define DIO0        26

#define FREQ        868E6           //Signal frequency
#define SPR_FACT    7               //Spreading factor
#define BANDWIDTH   250E3           //Bandwidth
//#define SYNC_WORD 0x00            //Syncronization byte
//#define PREAM_LEN 8               //Signal preamble length

#define IN_BUFFER_SIZE      255U    //Maximum number of bytes per input packet

#define GATEWAY_ID          0x53AC  //ID that all emmiters must use to communicate with gateway
#define GATEWAY_ID_LEN      2U      //Number of bytes that compose the gateway ID

#define ACK_TIMEOUT         5000U   //Acknowledgment timeout (ms)

#endif //LoRaCfg_H
