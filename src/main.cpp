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

#include <Wire.h>

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

  if(collectionConfig())
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

        Serial.printf("Bin %d is at %d%% of capacity.\n", in_packet[GATEWAY_ID_LEN], in_packet[GATEWAY_ID_LEN+2U]);
        //(void)uploadValue("new_value", new_value);

        if(current_cluster_update_flag)
        {
          if(updateCurrentCluster() != 1U)
          {
            current_cluster_update_flag = 0U;
          }
        }
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

  if(collectedClusterManager() == 1U)
  {
    current_cluster_update_flag = 1U;
  }

  if(!positionUpdateManager(&gps_latitude, &gps_longitude))
  {
    float64 distance;
    uint8 cluster = getCurrentCluster();
    if(!distanceToCluster(cluster, &distance))
    {
      Serial.printf("Next cluster %d is %.0f m away.\n", cluster, distance);
    }
  }
}
