#ifndef collection_H
#define collection_H

#include <platformTypes.h>

#include <collectionCfg.h>

uint8 findBin(uint8 bin_id);
uint8 saveBinFullness(uint8 bin_id, uint8 fullness);
bool isBinFullnessUpdated(uint8 bin_id);

#endif //collection_H
