#include <Arduino.h>

#include <customUtilities.h>

//Prints a uint8 string of len length. Returns 0 if successful, 1 if len = 0
uint8 printStr(uint8* str, uint16 len)
{
  if(len)
  {
    Serial.print("| ");
    for (uint16 w = 0; w < len; w++)
    {
      Serial.print(str[w]);
      Serial.print(" | ");
    }
    return 0;
  }
  else return 1;
}

//Performs a software reset after warning through the serial output and doing a countdown.
void SwReset(uint8 countdown)
{
  Serial.print("Reseting the MCU in ");
  for (uint8 w=countdown;w>0;w--)
  {
    Serial.print(w);
    Serial.print(" ");
    delay(1000);
  }
  ESP.restart();
}
