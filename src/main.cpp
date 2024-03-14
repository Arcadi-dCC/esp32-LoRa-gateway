#include <Arduino.h>
#include <platformTypes.h>

#include <customUtilities.h>
#include <influxDbClientPrivate.h>
#include <LoRaPrivate.h>
#include <mailClientPrivate.h>
#include <timePrivate.h>
#include <WiFiPrivate.h>

uint16 new_value = 31;
uint16 top_value = 170;

//RTC_DATA_ATTR uint16 ack_fails = 0;
//RTC_DATA_ATTR uint16 cldtime_fails = 0;
//RTC_DATA_ATTR uint16 unexpected_num_bytes = 0;
//RTC_DATA_ATTR uint16 duplicated_data = 0;

uint16 gateway_time_upd = 0;
uint16 emitter_time_upd = 0;

String first_part = "Pin 13's ADC value of ";
String third_part = "has been reached.";
String final_string ="";

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
  sensor.addTag("test", "LoRa_5minutes");
  sensor.addTag("try", "20240312_1");

  //Configure and log into e-mail account
  if (EmailConfig())
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

    case (GATEWAY_ID_LEN + 4U):
    {
      if(replyAck())
      {
        Serial.println("Failed to reply with acknowledgement");
        //ack_fails++;
      }

      if(isDataDuplicated())
      {
        Serial.println("Received data was duplicated");
        //duplicated_data++;
      }
      else
      {
        new_value = *((uint16*)(&in_packet[GATEWAY_ID_LEN + 2U]));
        Serial.print("Received value: ");
        Serial.println(new_value);

        (void)uploadValue("new_value", new_value);

        if (new_value > top_value)
        {
          top_value = new_value;
          final_string = first_part + top_value + third_part;
          EmailSend("New top value", final_string);
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
          //cldtime_fails++;
        }
        else
        {
          emitter_time_upd++;
          (void)uploadValue("emitter_time_upd", emitter_time_upd);
        }
        break;
      }
    }

    default:
    {
      Serial.println("Received packet had an unexpected number of bytes");
      //unexpected_num_bytes++;
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

  if(checkTimeUpdate() == 1U)
  {
    gateway_time_upd++;
    (void)uploadValue("gateway_time_upd", gateway_time_upd);
  }
}
