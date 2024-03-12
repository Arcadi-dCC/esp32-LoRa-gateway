#ifndef timeCfg_H
#define timeCfg_H

//Search your string by country at:
//https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

#define TCUPD_TIMEOUT 20000U    //Time configuration update timeout (ms)
#define TUPD_PERIOD 1800U       //Period between time updates via WiFi (s)

#endif //timeCfg_H
