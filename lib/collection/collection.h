#ifndef collection_H
#define collection_H

#include <platformTypes.h>

#include <collectionCfg.h>

float64* findBin(uint8 bin_id);
uint8 saveBinFullness(uint8 bin_id, uint8 fullness);
bool isBinFullnessUpdated(uint8 bin_id);
uint8 nextCluster(uint8* next_cluster, uint8 active_bin_id);
uint8* fullBinsInCluster(uint8 cluster, uint16* number_of_bins);

#endif //collection_H
