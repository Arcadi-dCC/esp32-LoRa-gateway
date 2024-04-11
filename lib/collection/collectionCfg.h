#ifndef collectionCfg_H
#define collectionCfg_H

#include <platformTypes.h>

#define TOTAL_BINS 7U

uint8 findBin(uint8 bin_id);

extern float64 bins[TOTAL_BINS][6U];

#endif //collectionCfg_H
