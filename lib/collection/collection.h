#ifndef collection_H
#define collection_H

#include <platformTypes.h>

#include <collectionCfg.h>

uint8 collectionConfig(void);
float64* findBin(uint8 bin_id);
uint8 saveBinFullness(uint8 bin_id, uint8 fullness);
bool isBinFullnessUpdated(uint8 bin_id);
uint8 nextCluster(uint8* next_cluster, uint8 active_bin_id);
uint8* fullBinsInCluster(uint8 cluster, uint16* number_of_bins);
uint8 clusterState(uint8 cluster_id);
uint8 collectedClusterManager(void);
void IRAM_ATTR onClusterCollected(void);
uint8 updateCurrentCluster(void);

extern uint8 cluster_collected_flag;

#endif //collection_H
