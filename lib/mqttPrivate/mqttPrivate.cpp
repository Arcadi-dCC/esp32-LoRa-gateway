#include <platformTypes.h>
#include <Arduino.h>

#include <mqttCfg.h>
#include <mqttPrivate.h>

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <collection.h>

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, espClient);

uint32 last_reconnect_attempt = 0;

//Tries to connect to MQTT server. Returns: 0 if connection successful, 1 if MQTT_TIMEOUT reached.
uint8 mqttConnect(void)
{
    Serial.print("Connecting to MQTT server");
    uint32 start_time = millis();

    while(!client.connected() && ((millis() - start_time) < MQTT_TIMEOUT))
    {
        client.connect(MQTT_CLIENT_ID);
        Serial.print(".");
        delay(MQTT_COOLDOWN);
    }
    Serial.println();

    if (millis() - start_time >= MQTT_TIMEOUT)
    {
        Serial.println("Could not connect to MQTT server (timeout reached).");
        return 1;
    }
    else return 0;
}

//Tries to publish the fullness and cluster of a specific bin in the MQTT server.
//It also attaches the gateway ID as tag.
//Returns 0 if successful, 1 if connection is cooldown mode, 2 if MQTT server could not be reached,
//3 if bin_id does not exist in CollectionCfg.cpp, 4 if bin fullnes isn't updated, 5 if sending stage failed.
uint8 mqttPublish(uint8 bin_id)
{
    //If connection has been lost, try to reconnect once every MQTT_COOLDOWN (ms)
    if(!client.connected())
    {
        if((millis() - last_reconnect_attempt) > MQTT_COOLDOWN)
        {
            last_reconnect_attempt = millis();
            Serial.println("Reconnecting to MQTT server.");
            client.connect(MQTT_CLIENT_ID);
        }
        else return 1;
    }

    //If there is connection, either after reconnecting or not
    if(client.connected())
    {
        last_reconnect_attempt = 0;

        //Retrieve data
        float64* bin_data = findBin(bin_id);
        if( bin_data == NULL)
        {
            Serial.println("MQTT data retrieve err: bad bin ID.");
            return 3;
        }
        if(bin_data[4] == 0xFF)
        {
            Serial.println("MQTT data retrieve err: bin fullness not updated.");
            return 4;
        }

        //Prepare message string in InfluxDB format
        char msg[MQTT_MSG_LEN];
        snprintf(msg, MQTT_MSG_LEN - 1, "bin%d,truck=%d,cluster=%d fullness=%d", (uint8)bin_data[0], 0, (uint8)bin_data[1], (uint8)bin_data[4]);
        /*TODO - Assign a real value to each truck via a Cfg.h file*/
        
        //Publish
        if(!client.publish(MQTT_TOPIC, msg))
        {
            Serial.println("Failed to publish message in MQTT server.");
            return 5;
        }
    }
    else return 2;

    return 0;
}

//Makes is easier to keep the connection with the MQTT server up.
//Calls client.loop() without having to include PubSubClient library elsewhere.
void mqttKeepCon(void) {client.loop();}
