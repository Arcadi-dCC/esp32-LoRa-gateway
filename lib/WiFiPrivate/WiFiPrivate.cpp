#include <Arduino.h>

#include <WiFiPrivate.h>
#include <WiFiCfg.h>

#include <WiFiMulti.h>
WiFiMulti wifiMulti;

//Tries to connect to WiFi. Returns: 0 if connection successful, 1 if 30 s timeout reached.
uint8 WiFiConnect(void)
{
  delay(1000);
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  byte connection_timer = 0;
  while ((wifiMulti.run() != WL_CONNECTED) and (connection_timer < 19)) {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (connection_timer > 18)
  {
    Serial.println("Could not connect to WiFi (timeout reached).");
    return 1;
  }
  else return 0;
}
