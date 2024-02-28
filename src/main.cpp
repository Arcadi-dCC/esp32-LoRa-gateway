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
RTC_DATA_ATTR uint16 not_four_bytes = 0;
RTC_DATA_ATTR uint16 missed_value = 0;
RTC_DATA_ATTR uint16 repeated_value = 0;

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
    if(replyAck())
    {
      Serial.println("Failed to reply with acknowledgement");
      ack_fails++;
    }
    Serial.print("Received string: ");
    printStr((uint8*)in_packet, in_packet_len);
    Serial.println();
    Serial.print("With length: ");
    Serial.println(in_packet_len);
   
    if(in_packet_len == 4)
    {
      old_value = new_value;
      new_value = in_packet[3];

      if (((old_value + 1) % 32) != new_value)
      {
        missed_value++;
      }

      if(old_value == new_value)
      {
        repeated_value++;
        //(void)EmailSend("20240227_1 test", "An email is sent because a repeated value has been received.");
      }
      
    }
    else
    {
      Serial.println("More than 4 bytes received");
      not_four_bytes++;
    }
    Serial.println();
    //(void)uploadValue("received_value", new_value);
    in_packet_len = 0;
  }
}
