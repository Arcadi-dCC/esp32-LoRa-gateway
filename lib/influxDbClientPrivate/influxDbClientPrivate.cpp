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
  uint32 start_time = millis();

  while ((!client.validateConnection()) and ((millis() - start_time) < INFLUXDB_CON_TIMEOUT))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if ((millis() - start_time) >= INFLUXDB_CON_TIMEOUT)
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

  uint32 start_time = millis();
  while ((!client.writePoint(sensor)) and ((millis() - start_time) < INFLUXDB_UPL_TIMEOUT))
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  if ((millis() - start_time) >= INFLUXDB_UPL_TIMEOUT)
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
