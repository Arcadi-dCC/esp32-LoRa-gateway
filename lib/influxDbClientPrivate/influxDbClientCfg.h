#ifndef influxDbClientCfg_H
#define influxDbClientCfg_H

#define INFLUXDB_URL        "http://10.20.255.254:8086/"
#define INFLUXDB_TOKEN      "RjBjj-Q5BDBV5ndi3_GNqEwc2EX8KDcErqzsBHk_Rf5t5QMVX1FFiYOcPsMGJYrZZsQw-W8fywxJR7Pb5A7JTw=="
#define INFLUXDB_ORG        "1d5669401414b39c"
#define INFLUXDB_BUCKET     "esp32_tests"

#define _MEASUREMENT        "LoRa"

#define INFLUXDB_CON_TIMEOUT    60000U      //InfluxDB connection to server timeout (ms)
#define INFLUXDB_UPL_TIMEOUT    10000U      //Data upload to InfluxDB server timeout (ms)

#endif //influxDbClientCfg_H
