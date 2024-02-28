#include <Arduino.h>
#include <platformTypes.h>

#include <customUtilities.h>
#include <influxDbClientPrivate.h>
#include <LoRaPrivate.h>
#include <mailClientPrivate.h>
#include <WiFiPrivate.h>

RTC_DATA_ATTR uint8 new_value = 31;
RTC_DATA_ATTR uint8 old_value = 0;

RTC_DATA_ATTR uint16 ack_fails = 0;
RTC_DATA_ATTR uint16 not_five_bytes = 0;
RTC_DATA_ATTR uint16 missed_data = 0;
RTC_DATA_ATTR uint16 duplicated_data = 0;

RTC_DATA_ATTR uint8 previous_did = 0xFF;

void setup() {

  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Gateway");

  //Connect to WiFi
  //if (WiFiConnect())
  //{
  //  SwReset(10);
  //}

  //Connect to InfluxDB server
  //if (InfluxServerConnect())
  //{
  //  SwReset(10);
  //}

  //Add tags
  //sensor.addTag("test", "LoRa_2minutes");
  //sensor.addTag("try", "20240227_1");

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
  if(in_packet_len)
  {
    if(in_packet_len == 5)
    {
      if(replyAck())
      {
        Serial.println("Failed to reply with acknowledgement");
        ack_fails++;
      }

      if(in_packet[3] != previous_did)
      {
        old_value = new_value;
        new_value = in_packet[4];

        if (((old_value + 1) % 32) != new_value)
        {
          missed_data++;
        }

        previous_did = in_packet[3];

        //(void)uploadValue("received_value", new_value);
      }
      else
      {
        Serial.println("Received data was duplicated");
        duplicated_data++;
      }
    }
    else
    {
      Serial.println("Not 5 bytes received");
      not_five_bytes++;
    }

    Serial.print("Received string: ");
    printStr((uint8*)in_packet, in_packet_len);
    Serial.println();
    Serial.print("With length: ");
    Serial.println(in_packet_len);
    Serial.println();

    in_packet_len = 0;
  }
}
