#ifndef gpsPrivate_H
#define gpsPrivate_H

#include <platformTypes.h>

uint8 gpsConfig(void);
uint8 checkPositionUpdate(float64* latitude, float64* longitude);

#endif //gpsPrivate_H