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

//Reads GPS messages and gives the last updated GPS position of the device through reference.
//If position is not updated, the reference variables are not modified.
//Returns 0 if a message has been successfully decoded, 1 if no new character, 2 if invalid message.
uint8 getGpsPosition(float64* lat, float64* lng)
{
    uint8 returner = 1U;

    while(ss.available() > 0)
    {
        if(gps.encode(ss.read()))
        {
            //reached only if the end of a message has been received.
            if(gps.location.isValid())
            {
                *lat = gps.location.lat();
                *lng = gps.location.lng();
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

//Calls getGpsPosition every time to handle UART connection with GPS module.
//Checks if GPS_UPD_PERIOD time has passed since the last GPS update.
//If so, it checks if getGpsPosition has decoded a new position since last update, and gives it through reference values.
//Returns  0 if position has been updated, 1 if update was not necessary, 2 if position could not be updated. 
uint8 positionUpdateManager(float64* lat, float64* lng)
{
    static uint32 last_updated = millis();
    static float64 pos[2] = {0, 0}; //latitude, longitude
    static uint8 new_pos = false;

    if(!getGpsPosition(&pos[0], &pos[1]))
    {
        new_pos = true;
    }
    
    if(millis() - last_updated < GPS_UPD_PERIOD)
    {
        return 1U;
    }
    else
    {
        last_updated = millis();
        if(new_pos)
        {
            *lat = pos[0];
            *lng = pos[1];
            new_pos = false;
            return 0U;
        }
        else
        {
            Serial.println("GPS position was not updated in the last seconds.");
            return 2U;
        }
    }
}
