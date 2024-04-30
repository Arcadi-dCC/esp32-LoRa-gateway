#ifndef gpsPrivate_H
#define gpsPrivate_H

#include <platformTypes.h>

uint8 gpsConfig(void);
uint8 positionUpdateManager(float64* latitude, float64* longitude);
uint8 distanceToCluster(uint8 cluster_id, float64* distance);

#endif //gpsPrivate_H
