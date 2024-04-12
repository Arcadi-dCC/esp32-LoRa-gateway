#include <Arduino.h>

#include <collection.h>

//Looks for the file containing info of the specified bin, by looking for a matching bin ID.
//Returns the file with the desired bin info, or 0xFF if the given bin_id was not found in the database.
uint8 findBin(uint8 bin_id)
{
    uint8 i = 0U;
    do
    {
        if(bins[i][0U] == (float64)bin_id)
        {
            break;
        }
        i++;
    } while (i < TOTAL_BINS);

    if(i<TOTAL_BINS)
    {
        return i;
    }

    return 0xFF;
}

//Stores the fullness of the specified bin.
//Returns 0 if successful, 1 if bin id was not found in the database.
uint8 saveBinFullness(uint8 bin_id, uint8 fullness)
{
    uint8 i = findBin(bin_id);
    if(i == 0xFF)
    {
        return 1;
    }

    bins[i][4U] = (float64)fullness;
    return 0;
}

//Looks up if the fullness value of a specified bin has been updated sincs last restart.
bool isBinFullnessUpdated(uint8 bin_id)
{
    uint8 i = findBin(bin_id);
    if(i == 0xFF)
    {
        return true;
    }

    if(bins[i][4U] == (float64)0xFF)
    {
        return false;
    }
    return true;
}