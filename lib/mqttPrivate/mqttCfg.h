#ifndef mqttCfg_H
#define mqttCfg_H

#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "brossesURV/tarragona"

#define MQTT_CLIENT_ID "brossesURV"

#define MQTT_MSG_LEN 50                 //Maximum length of the message to be published in the MQTT server"

#define MQTT_TIMEOUT    30000U          //Connection to MQTT server timeout (ms)
#define MQTT_COOLDOWN   1000U           //Cooldown between MQTT server reconnection attempts (ms)

#endif // mqttCfg_H
