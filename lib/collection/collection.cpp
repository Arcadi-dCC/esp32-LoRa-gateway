#include <Arduino.h>

#include <collection.h>

uint8 current_cluster, previous_cluster;

//Looks for the file containing info of the specified bin, by looking for a matching bin ID.
//Returns the a pointer the desired bin info, or NULL if the given bin_id was not found in the database.
float64* findBin(uint8 bin_id)
{
    uint8 i = 0U;
    do
    {
        if(bins[i][0U] == (float64)bin_id)
        {
            return bins[i];
        }
        i++;
    } while (i < TOTAL_BINS);

    return NULL;
}

//Stores the fullness of the specified bin.
//Returns 0 if successful, 1 if bin id was not found in the database.
uint8 saveBinFullness(uint8 bin_id, uint8 fullness)
{
    float64* bin_info = findBin(bin_id);
    if(bin_info == NULL)
    {
        return 1U;
    }

    bin_info[4U] = (float64)fullness;
    return 0U;
}

//Looks up if the fullness value of a specified bin has been updated since last restart.
bool isBinFullnessUpdated(uint8 bin_id)
{
    float64* bin_info = findBin(bin_id);
    if(bin_info == NULL)
    {
        return 1U;
    }

    if(bin_info[4U] == (float64)0xFF)
    {
        return false;
    }
    return true;
}

//Looks for the next cluster and returns it through reference variable.
//Needs to know the id of the cluster that has just been collected.
//Returns 0 if successful, 1 if active cluster does not exist, 2 if active cluster is the last cluster
uint8 nextCluster(uint8* next_cluster, uint8 active_cluster_id)
{
    uint8 i = 0U;
    //Look for the active cluster
    do
    {
        if(bins[i][1] == (float64)active_cluster_id)
        {
            break;
        }
        i++;
    } while (i < TOTAL_BINS);
    if(i >= TOTAL_BINS)
    {
        return 1U; //active cluster id does not exist in bins table
    }
    //Go to the end of the active cluster
    do
    {
        if(bins[i][1] != (float64)active_cluster_id)
        {
            break;
        }
        i++;
    } while (i < TOTAL_BINS);
    if(i >= TOTAL_BINS)
    {
        return 2U; //this was the last cluster
    }

    *next_cluster = (uint8)(bins[i][1U]);
    return 0U;
}

//Returns the bins that need to be collected in a cluster.
//It does so by returning a variable-sized array of length number_of_bins.
//The returned pointer must be deleted with delete[] pointer to avoid a memory leak.
//If the cluster does not exist, there are no full bins, or memory could not be allocated, it returns NULL and number_of_bytes = 0.
uint8* fullBinsInCluster(uint8 cluster, uint16* number_of_bins)
{
    uint8 i = 0U;
    do
    {
        if(bins[i][1U] == (float64)cluster)
        {
            break;
        }
        i++;
    } while(i < TOTAL_BINS);
    
    *number_of_bins = 0U;
    if(i < TOTAL_BINS)
    {
        uint8 temp_list[TOTAL_BINS];

        while(bins[i][1U] == (float64)cluster)
        {
            if((bins[i][4] >= MIN_FULLNESS) and (bins[i][4] != 0xFF))
            {
                temp_list[*number_of_bins] = (uint8)(bins[i][0]);
                (*number_of_bins)++;
            }
            i++;
        }
        uint8* list = new uint8[*number_of_bins];
        if(list)
        {
            for(i = 0U; i < *number_of_bins; i++)
            {
                list[i] = temp_list[i];
            }
        }
        else
        {
            *number_of_bins = 0U;
        }
        return list;
    }
    return NULL;
}

bool isClusterUpdated(uint8 cluster)
{
    float64* cluster_info = findCluster(cluster);
    uint8 i = 0U;
    while(cluster_info[])
}

//Sorts out the state of the cluster.
//returns 0 if the state of any bin in the cluster is not known, 1 if there is no full bin, 2 if there are full bins
uint8 clusterState(uint8 cluster_id)
{
    if(!isClusterUpdated)
    {
        return 0U;
    }
    uint16 full_bins = 0U;
    uint8* full_bin_list = fullBinsInCluster(cluster_id, &full_bins);

    uint8 returner = 1U;
    if(full_bin_list != NULL)
    {
        returner++;
    }
    delete[] full_bin_list;
    return returner;
}

void clusterWasCollected(void)
{
    while(1)
    {
        previous_cluster = current_cluster;
        (void)nextCluster(&current_cluster, previous_cluster);
        switch (clusterState(current_cluster))
        {
        case (0U): /*Some bins in cluster are not yet updated*/
        {
            //PrintScr("Follow the route");
            break;
        }
        case (1U): /*No full bins in cluster*/
        {
            continue;
        }
        case (2U): /*Some full bins in cluster*/
        {
            break;
        }
        default:
        {
            /*Do nothing*/
            break;
        }
        }
    }
}
