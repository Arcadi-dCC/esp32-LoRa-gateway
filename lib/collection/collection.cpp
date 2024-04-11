#include <Arduino.h>

#include <collectionCfg.h>

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