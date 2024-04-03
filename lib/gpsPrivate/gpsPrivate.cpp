#include <Arduino.h>

#include <gpsPrivate.h>
#include <gpsCfg.h>

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;
SoftwareSerial ss(RX_PIN, TX_PIN); //RxPin, TxPin

//Configures the UART communication with the GPS module.
//Always returns 0: successful.
uint8 gpsConfig(void)
{
    ss.begin(BAUD_RATE);
    return 0;
}

//Gives the last updated GPS position of the device through refernces.
//Returns 0 if successful, 1 if no new message, 2 if invalid message.
uint8 getGpsPosition(float64* latitude, float64* longitude)
{
    uint8 returner = 1U;

    while(ss.available() > 0)
    {
        if(gps.encode(ss.read()))
        {
            //reached only if the end of a message has been received.
            if(gps.location.isValid())
            {
                *latitude = gps.location.lat();
                *longitude = gps.location.lng();
                returner = 0U;
            }
            else
            {
                returner = 2U;
            }
        }
    }
    return returner;
}
