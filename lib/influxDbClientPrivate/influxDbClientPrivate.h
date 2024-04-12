#ifndef influxDbClientPrivate_H
#define influxDbClientPrivate_H

#include <platformTypes.h>
#include <InfluxDbClient.h>

uint8 InfluxServerConnect(void);
uint8 uploadValue(const String &field, uint8 value);
uint8 uploadValue(const String &field, float64 value, uint8 decimal_places);

extern Point sensor; // Data point (_measurement)

#endif //influxDbClientPrivate_H
