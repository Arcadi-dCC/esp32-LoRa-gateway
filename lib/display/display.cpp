#include <Arduino.h>

#include <display.h>
#include <displayCfg.h>

#include <collection.h>
#include <gpsPrivate.h>

#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield display = Adafruit_RGBLCDShield();

/*
SCREEN 0 ---> Distance to next cluster.
SCREEN 1 ---> Collect certain bins.
SCREEN 2 ---> Follow the route.
SCREEN 3 ---> All bins have been collected.
*/
uint8 screen = 2U;

const char* messages[4] = {MSG_0, MSG_1, MSG_2, MSG_3};
const uint32 timeouts[4] = {MSG_0_TOUT, MSG_1_TOUT, MSG_2_TOUT, MSG_3_TOUT};

uint16 next_bins[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

//Initializes he display and cleans it.
//Alwas returns 0 (successful).
uint8 displayConfig(void)
{
    display.begin(DSP_WIDTH, DSP_HEIGHT);
    display.clear();
    display.printf("Starting...");
    return 0U;
}

void showScreen()
{
    display.clear();
    display.setCursor(0,0);
    display.printf(messages[screen]);

    display.setCursor(0,1);
    switch (screen)
    {
        case (0U):
        {
            float64 distance;
            if(!distanceToCluster(getCurrentCluster(), &distance))
            {
                if(distance < 1000U)
                {
                    display.printf("%.0f m", distance);
                }
                else
                {
                    distance = distance / (float64)1000;
                    display.printf("%.1f km", distance);
                }
            }
            else
            {
                display.printf("Unknown");
            }
            break;
        }
        case (1U):
        {
            uint16 full_bins = 0U;
            uint8* full_bins_list = fullBinsInCluster(getCurrentCluster(), &full_bins);

            uint16 i = 0U;
            for(i = 0U; i < full_bins; i++)
            {
                display.print(full_bins_list[i]);
                display.printf(" ");
            }
            delete[] full_bins_list;
            break;
        }
        default:
        {
            break;
        }
    }
}

void screenSequencer(void)
{
    static uint32 last_time_upd = 0U;
    static uint8 shownScreen = 4U;

    if(timeouts[screen] > 0U)
    {
        if(millis() - last_time_upd > timeouts[screen])
        {
            last_time_upd = millis();
            screen = (screen + 1U) % 2U;
            showScreen();
            shownScreen = screen;
        }
    }
    else
    {
        if(shownScreen != screen)
        {
            showScreen();
            shownScreen = screen;
        }
    }
}