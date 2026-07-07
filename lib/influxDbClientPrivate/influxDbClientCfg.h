#ifndef influxDbClientCfg_H
#define influxDbClientCfg_H

#define INFLUXDB_URL        "http://192.168.1.254:8086/"
#define INFLUXDB_TOKEN      "1Dgu28vOIGs54s4ZDGrzXxNhDxG9PzB-x11YNHKa2cd7VFe06cYBv92ZG9bZOqM__sbJ3ngO9lo4G7yXElxQXw=="
#define INFLUXDB_ORG        "fb0b71fa22c2cd78"
#define INFLUXDB_BUCKET     "truck0"

#define _MEASUREMENT        "fullness"

#define INFLUXDB_CON_TIMEOUT    60000U      //InfluxDB connection to server timeout (ms)
#define INFLUXDB_UPL_TIMEOUT    10000U      //Data upload to InfluxDB server timeout (ms)

#endif //influxDbClientCfg_H
