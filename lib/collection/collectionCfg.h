#ifndef collectionCfg_H
#define collectionCfg_H

#include <platformTypes.h>

#define TOTAL_BINS      7U
extern float64 bins[TOTAL_BINS][6U];

#define MIN_FULLNESS    50U //%

uint8 findBin(uint8 bin_id);
uint8 saveBinFullness(uint8 bin_id, uint8 fullness);
bool isBinFullnessUpdated(uint8 bin_id);

#endif //collectionCfg_H
