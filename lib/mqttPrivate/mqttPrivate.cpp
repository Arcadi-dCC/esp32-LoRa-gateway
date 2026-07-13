#include <platformTypes.h>
#include <Arduino.h>

#include <mqttCfg.h>
#include <mqttPrivate.h>

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <collection.h>

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, espClient);

uint8 mqttPubBuf[MQTT_BUF];
uint8 bufIndex = 0U;
uint8 bufLast = 0U;

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
    else
    {
        //Initialize some variables and return OK
        bufIndex = 0U;
        bufLast = 0U;
        return 0;
    }
}

//Adds a bin to the FIFO buffer of bins whose data must be sent to the MQTT server.
//Always leaves one free slot between last data added and current index.
//Returns 0 if successful, 1 if buffer is full.
uint8 mqttAppendBin(uint8 bin_id)
{
    if((bufLast+1) % MQTT_BUF == bufIndex) return 1; //buffer is full

    mqttPubBuf[bufLast] = bin_id;
    bufLast = (bufLast + 1) % MQTT_BUF;
    return 0;
}

//Dequeues the oldest bin from the FIFO buffer of bins whose data must be sent to the MQTT server.
//Retrieves its value per parameter.
//Returns 0 if successful, 1 if buffer is empty
uint8 mqttDequeueBin(uint8 *bin_id)
{
    if(bufIndex == bufLast) return 1; //buffer is empty
    
    *bin_id = mqttPubBuf[bufIndex];
    bufIndex = (bufIndex + 1) % MQTT_BUF;
    return 0;
}

//Reads the oldest bin from he FIFO buffer of bins, but doesn't dequeue it.
//Returns 0 if successful, 1 if buffer is empty
uint8 mqttReadBin(uint8 *bin_id)
{
    if(bufIndex == bufLast) return 1; //buffer is empty
    
    *bin_id = mqttPubBuf[bufIndex];
    return 0;
}

//Tries to publish the fullness and cluster of the oldest bin appended in the publish buffer (FIFO).
//It also attaches the gateway ID as tag. The bin is deleted from the buffer only if publishing was successful.
//Returns 0 if successful, 1 if buffer is empty, 2 if connection is in cooldown mode, 3 if MQTT server could not be reached,
//4 if bin id does not exist in CollectionCfg.cpp, 5 if bin fullnes isn't updated, 6 if sending stage failed.
uint8 mqttPublishMgr(void)
{
    static uint32 last_interaction = 0U;

    //Makes is easier to keep the connection with the MQTT server up.
    //Called everytime. Put here to have a cleaner code in main.
    client.loop();

    //Read what the next bin in the FIFO buffer is, if any, before proceeding
    uint8 bin_id = 0U;
    if(mqttReadBin(&bin_id)) return 1;

    //Interact with the MQTT server only after the cooldown time.
    if((millis() - last_interaction) < MQTT_COOLDOWN) return 2;
    
    last_interaction = millis();

    //If connection has been lost, try to reconnect once.
    if(!client.connected())
    {
        Serial.println("Reconnecting to MQTT server.");
        client.connect(MQTT_CLIENT_ID);
    }

    //Abort if MQTT server couldn't be reached
    if(!client.connected()) return 3;
        
    //Retrieve bin data
    float64* bin_data = findBin(bin_id);
    if(bin_data == NULL)
    {
        Serial.println("MQTT data retrieve err: bad bin ID.");
        return 4;
    }
    if(bin_data[4] == 0xFF)
    {
        Serial.println("MQTT data retrieve err: bin fullness not updated.");
        return 5;
    }

    //Prepare message string in InfluxDB format
    char msg[MQTT_MSG_LEN];
    snprintf(msg, MQTT_MSG_LEN - 1, "bin%d,truck=%d,cluster=%d fullness=%d", (uint8)bin_data[0], 0, (uint8)bin_data[1], (uint8)bin_data[4]);
    /*TODO - Assign a real value to each truck via a Cfg.h file*/
        
    //Try to publish
    if(!client.publish(MQTT_TOPIC, msg))
    {
        Serial.println("Failed to publish message in MQTT server.");
        return 6;
    }

    //Pop the oldest bin from buffer after successful publishing
    mqttDequeueBin(&bin_id);
    return 0;
}
