#include <Arduino.h>

#include <timePrivate.h>
#include <timeCfg.h>

RTC_DATA_ATTR time_t last_updated;

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
        time(&last_updated);
        Serial.print("Time is: ");
        Serial.print(ctime(&last_updated));
        return 0;
    }
    else
    {
        Serial.println("Could not update time (timeout reached).");
        return 1;
    }
}

//Looks up the last calendar time that time was updated and compares with current calendar time.
//If the difference is greater than TUPD_PERIOD, it calls timeConfigWiFi to update time.
//Returns 0 if update was not necessary, 1 if time has been updated, 2 if error. 
uint8 checkTimeUpdate(void)
{
    time_t now;
    time(&now);
    if(now - last_updated < TUPD_PERIOD)
    {
        return 0;
    }
    else
    {
        return (timeConfigWiFi() + 1U);
    }
}

//Puts the MCU to sleep, and wakes it up again after the specified amount of time has passed.
void sleepFor(uint16 seconds, uint16 minutes, uint16 hours, uint16 days)
{
  uint64 total_time_us = 0;

  total_time_us += 86400U * (uint64)days;
  total_time_us += 3600U * (uint64)hours;
  total_time_us += 60U * (uint64)minutes;
  total_time_us += (uint64)seconds;
    
  total_time_us *= 1000000U; //conversion to us
  Serial.print("Sleeping for ");
  Serial.print(total_time_us / 1000000U);
  Serial.println(" s");
  esp_deep_sleep(total_time_us);
}

//Puts the MCU to sleep, and wakes it up again at the specified date and time.
//Returns 1 if specified date is not in the future.
uint8 sleepUntil(int year, int month, int day, int hour, int minute, int second)
{
  //insert date in tm struct with expected format
  struct tm wakeup_date;

  wakeup_date.tm_year = year - 1900;
  wakeup_date.tm_mon = month - 1;
  wakeup_date.tm_mday = day;
  wakeup_date.tm_hour = hour;
  wakeup_date.tm_min = minute;
  wakeup_date.tm_sec = second;
  wakeup_date.tm_isdst = -1;

  time_t wakeup_cldtime = mktime(&wakeup_date); //get calendar time of wake up date
  time_t current_cldtime;
  time(&current_cldtime); //get current calendar time

  if (wakeup_cldtime > current_cldtime)
  {
    //calculate number of us left for reaching wake up date
    uint64 total_time_us = ((uint64)(wakeup_cldtime - current_cldtime)) * 1000000U;
    Serial.print("Waking up on ");
    Serial.print(asctime(&wakeup_date));

    Serial.print("Sleeping for ");
    Serial.print(total_time_us / 1000000U);
    Serial.println(" s");
    esp_deep_sleep(total_time_us);
  }
  else
  {
    Serial.println("Wake up date is not in the future");
    return 1;
  }
}
