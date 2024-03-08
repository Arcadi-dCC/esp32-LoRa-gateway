#include <Arduino.h>

#include <timePrivate.h>
#include <timeCfg.h>

//Connects to external server to get UTC calendar time. Then, updates the time zone and prints the time. Timeout of TCUPD_TIMEOUT
//Returns 0 if successful, 1 if error.
uint8 timeConfigWiFi(void)
{
    Serial.print("Configuring local time");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm time_info;

    bool err_reg = false;
    uint32 start_time = millis();
    do
    {
        err_reg = getLocalTime(&time_info, 1000U);
        Serial.print(".");
    } while((!err_reg) and ((millis() - start_time) < TCUPD_TIMEOUT));
    Serial.println();

    if(err_reg)
    {
        setenv("TZ", TZ_INFO, 1);
        tzset();
        time_t cldtime;
        time(&cldtime);
        Serial.print("Time is: ");
        Serial.print(ctime(&cldtime));
        return 0;
    }
    else
    {
        Serial.println("Could not update time (timeout reached).");
        return 1;
    }
}
