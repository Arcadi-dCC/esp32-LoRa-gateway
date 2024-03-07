#include <Arduino.h>

#include <influxDbClientPrivate.h>
#include <influxDbClientCfg.h>

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensor(_MEASUREMENT); // Data point (_measurement);

//Connects to the InfluxDB server. Returns: 0 if connection successful, 1 if error.
uint8 InfluxServerConnect(void)
{
  Serial.print("Connecting to InfluxDB local server");
  int returner = 0;
  byte connection_timer = 0;

  while ((!client.validateConnection()) and (connection_timer < 2))
  {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (connection_timer > 1)
  {
    Serial.print("Timeout reached. InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    returner = 1;
  }

  else
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  return returner;
}

//Uploads a value to InfluxDB server with specified field. Returns: 0 if successful, 1 if error.
uint8 uploadValue(const String &field, uint8 value)
{
  Serial.print("Trying to upload ");
  Serial.print(field);
  Serial.print(" = ");
  Serial.print(value);
  Serial.print(" ");

  sensor.clearFields();
  sensor.addField(field, value);

  int upload_timer = 0;
  while ((!client.writePoint(sensor)) and (upload_timer < 2))
  {
    upload_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (upload_timer > 1)
  {
    Serial.print("Timeout reached. InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
    return 1;
  }
  else
  {
    return 0;
  }
}
