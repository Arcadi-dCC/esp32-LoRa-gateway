#include <Arduino.h>
#include <platformTypes.h>

#include <customUtilities.h>
#include <gpsPrivate.h>
#include <influxDbClientPrivate.h>
#include <LoRaPrivate.h>
#include <mailClientPrivate.h>
#include <timePrivate.h>
#include <WiFiPrivate.h>
#include <collection.h>

uint8 new_value = 0xFF;
float64 gps_latitude, gps_longitude;

uint8 gps_return;

void setup() {

  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Gateway");

  //Connect to WiFi
  if (WiFiConnect())
  {
    SwReset(10);
  }

  //Configure local time via WiFi
  if(timeConfigWiFi())
  {
    SwReset(10);
  }
  
  //Connect to InfluxDB server
  if (InfluxServerConnect())
  {
    SwReset(10);
  }

  //Add tags
  sensor.addTag("test", "GPS_5seconds");
  sensor.addTag("try", "20240404_3");

  //Configure and log into e-mail account
  if (EmailConfig())
  {
    SwReset(10);
  }

  //Configure connection with GPS module
  if(gpsConfig())
  {
    SwReset(10);
  }

  if (LoRaConfig())
  {
    SwReset(10);
  }
  delay(1000);
  Serial.println("Listening");
}

void loop(){

  switch(in_packet_len)
  {
    case 0U:
    {
      //Do nothing
    }break;

    case (GATEWAY_ID_LEN + 3U):
    {
      if(replyAck())
      {
        Serial.println("Failed to reply with acknowledgement");
      }
      //if(isDataDuplicated())
      //{
      //  Serial.println("Received data was duplicated");
      //}
      if(isBinFullnessUpdated(in_packet[GATEWAY_ID_LEN]))
      {
        Serial.println("Fullness of the sending bin was already updated.");
      }
      else
      {
        saveBinFullness(in_packet[GATEWAY_ID_LEN], in_packet[GATEWAY_ID_LEN+2U]);

        Serial.print("Received fullness: ");
        Serial.print(in_packet[GATEWAY_ID_LEN+2U]);
        Serial.print(" from bin: ");
        Serial.println(in_packet[GATEWAY_ID_LEN]);
        //(void)uploadValue("new_value", new_value);
      }
      break;
    }

    case (GATEWAY_ID_LEN + 1U):
    {
      if(in_packet[GATEWAY_ID_LEN] == CLDTIME_MSG_ID)
      {
        if(replyCalendarTime())
        {
          Serial.println("Failed to reply with calendar time");
        }
        break;
      }
    }

    default:
    {
      Serial.println("Received packet had an unexpected number of bytes");
    }
  }

  if(in_packet_len)
  {
    Serial.print("Received string: ");
    printStr((uint8*)in_packet, in_packet_len);
    Serial.println();
    Serial.print("With length: ");
    Serial.println(in_packet_len);
    Serial.println();

    in_packet_len = 0;
  }

  (void)timeUpdateManager();

  if(!positionUpdateManager(&gps_latitude, &gps_longitude))
  {
    Serial.print("New GPS position: ");
    Serial.print(gps_latitude, 5);
    Serial.print(" ");
    Serial.println(gps_longitude, 5);
  }
}
