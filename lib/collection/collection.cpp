#include <Arduino.h>

#include <collection.h>

#include <display.h>

uint8 previous_cluster, current_cluster = (uint8)bins[0U][1U];
volatile uint8 cluster_collected_flag = 0U;
uint8 current_cluster_update_flag = 1U;

//Configures collection done button as input, and configures the interrupt on such button.
//Always returns 0 (sucessful).
uint8 collectionConfig(void)
{
    pinMode(COL_DONE_BTN, INPUT);
    attachInterrupt(COL_DONE_BTN, onClusterCollected, RISING);
    Serial.println("Follow the route.");
    screen = 2U;
    return 0;
}

//Looks for the file containing info of the specified bin, by looking for a matching bin ID.
//Returns a pointer to the desired bin info, or NULL if the given bin_id was not found in the database.
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

//Sorts out the state of the cluster.
//Returns 0 if the state of the cluster is not fully updated, 1 if all bins are empty, 2 if there are full bins, 3 if cluster does not exist
uint8 clusterState(uint8 cluster_id)
{
    uint8 i = 0U;
    do
    {
        if(bins[i][1U] == (float64)cluster_id)
        {
            break;
        }
        i++;
    } while (i < TOTAL_BINS);

    if(i >= TOTAL_BINS)
    {
        return 3U;
    }

    uint16 full_bins = 0U;

    while((i < TOTAL_BINS) and (bins[i][1U] == (float64)cluster_id))
    {
        if(bins[i][4U] == (float64)0xFF)
        {
            return 0U;
        }
        else if(bins[i][4U] >= MIN_FULLNESS)
        {
            full_bins++;
        }
        i++;
    }
    if(full_bins)
    {
        return 2U;
    }

    return 1U;
}

void IRAM_ATTR onClusterCollected(void)
{
    cluster_collected_flag = 1U;
}

//Prints what bins the driver should collect next.
//Returns 0 if successful, 1 if next cluster state is not updated, 2 if all bins are already collected, 3 if update was not necessary, 4 if error
uint8 collectedClusterManager(void)
{
    if(!cluster_collected_flag)
    {
        return 3U;
    }

    cluster_collected_flag = 0U;
    while(1)
    {
        previous_cluster = current_cluster;

        switch(nextCluster(&current_cluster, previous_cluster))
        {
            case(0U):
            {
                switch(clusterState(current_cluster))
                {
                    case(0U): //Unknown state
                    {
                        Serial.print("Follow the route.\n");
                        screen = 2U;
                        return 1U;
                    }
                    case(1U): //No full bins
                    {
                        continue;
                    }
                    case(2U): //Some full bins
                    {
                        printFullBins();
                        Serial.println();
                        screen = 1U;
                        return 0U;
                    }
                    default:
                    {
                        /*Do nothing*/
                        break;
                    }
                }
                break;
            }
            case(2U): //This was the last cluster
            {
                Serial.print("All full bins have been collected.\n");
                screen = 3U;
                return 2U;
            }
            default: //Error
            {
                return 4U;
            }
        }
    }
}

//Checks if the current cluster is fully updated. If so, it lowers the flag to keep updating, and depending on the case:
//if here are full bins: it triggers collection message
//if all bins are empty: it raises the cluster_collected flag to call collectedClusterManager()
void clusterUpdateManager(void)
{
    switch(clusterState(current_cluster))
    {
        case(0U): //cluster is still not fully updated
        {
          break;
        }
        case(1U): //All bins are empty
        {
          cluster_collected_flag = 1U;
        }
        case(2U): //There are full bins
        {
          printFullBins();
          Serial.println();
          screen = 1U;
        }
        default: //cluster is updated
        {
          current_cluster_update_flag = 0U;
          break;
        }
    }
}

uint8 getCurrentCluster(void)
{
    return current_cluster;
}

//Prints the id's of the full bins in the current cluster
void printFullBins(void)
{
    uint16 full_bins = 0U;
    uint8* full_bins_list = fullBinsInCluster(current_cluster, &full_bins);

    uint16 i = 0U;
    Serial.print("Collect bins: ");
    for(i = 0U; i < full_bins; i++)
    {
        Serial.print(full_bins_list[i]);
        Serial.print(" ");
    }
    delete[] full_bins_list;
}
