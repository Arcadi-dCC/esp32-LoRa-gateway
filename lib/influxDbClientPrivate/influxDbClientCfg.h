#ifndef influxDbClientCfg_H
#define influxDbClientCfg_H

#define INFLUXDB_URL        "http://10.20.255.254:8086/"
#define INFLUXDB_TOKEN      "RjBjj-Q5BDBV5ndi3_GNqEwc2EX8KDcErqzsBHk_Rf5t5QMVX1FFiYOcPsMGJYrZZsQw-W8fywxJR7Pb5A7JTw=="
#define INFLUXDB_ORG        "1d5669401414b39c"
#define INFLUXDB_BUCKET     "esp32_tests"

//Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
//Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO             "CET-1CEST,M3.5.0,M10.5.0/3"

#define _MEASUREMENT        "LoRa"

#endif //influxDbClientCfg_H
