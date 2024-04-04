#ifndef gpsCfg_H
#define gpsCfg_H

#define RX_PIN          34      //Rx pin (from ESP32 perspective) connecting to GPS Module.
#define TX_PIN          12      //Tx pin (from ESP32 perspective) connecting to GPS Module.
#define BAUD_RATE       9600    //Baud rate of the UART connection to GPS module.
#define GPS_UPD_PERIOD  5000    //Time between GPS position updates (ms)

#endif //gpsCfg_H
