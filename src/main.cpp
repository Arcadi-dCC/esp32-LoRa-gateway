#include <Arduino.h>
#include <platformTypes.h>

#include <customUtilities.h>
#include <influxDbClientPrivate.h>
#include <LoRaPrivate.h>
#include <mailClientPrivate.h>
#include <timePrivate.h>
#include <WiFiPrivate.h>

RTC_DATA_ATTR uint16 new_value = 31;

RTC_DATA_ATTR uint16 ack_fails = 0;
RTC_DATA_ATTR uint16 cldtime_fails = 0;
RTC_DATA_ATTR uint16 unexpected_num_bytes = 0;
RTC_DATA_ATTR uint16 duplicated_data = 0;

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
  //if (InfluxServerConnect())
  //{
  //  SwReset(10);
  //}

  //Add tags
  //sensor.addTag("test", "LoRa_5minutes");
  //sensor.addTag("try", "20240308_1");

  //Configure and log into e-mail account
  //if (EmailConfig())
  //{
  //  SwReset(10);
  //}

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

    case (GATEWAY_ID_LEN + 4U):
    {
      if(replyAck())
      {
        Serial.println("Failed to reply with acknowledgement");
        ack_fails++;
      }

      if(isDataDuplicated())
      {
        Serial.println("Received data was duplicated");
        duplicated_data++;
      }
      else
      {
        new_value = *((uint16*)(&in_packet[GATEWAY_ID_LEN + 2U]));
        Serial.print("Received value: ");
        Serial.println(new_value);

        //(void)uploadValue("received_value", new_value);
        //(void)uploadValue("ack_fails", ack_fails);
        //(void)uploadValue("unexpected_num_bytes", unexpected_num_bytes);
        //(void)uploadValue("duplicated_data", duplicated_data);
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
          cldtime_fails++;
        }
        break;
      }
    }

    default:
    {
      Serial.println("Received packet had an unexpected number of bytes");
      unexpected_num_bytes++;
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

  (void)checkTimeUpdate();
}
