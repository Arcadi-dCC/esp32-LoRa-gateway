#include <Arduino.h>

#include <display.h>
#include <displayCfg.h>

#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield display = Adafruit_RGBLCDShield();

/*
SCREEN 1 ---> Distance to next cluster.
SCREEN 2 ---> Collect certain bins.
SCREEN 3 ---> Follow the route.
SCREEN 4 ---> All bins have been collected.
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
    display.print("Starting...");
    return 0U;
}

void showScreen()
{
    display.clear();
    display.setCursor(0,0);
    display.print(messages[screen]);

}

void screenSequencer(void)
{
    static uint32 last_time_upd = 0U;
    static uint8 upd_ct_msg_flag = 1U;

    if(timeouts[screen] > 0U)
    {
        upd_ct_msg_flag = 1U;
        if(millis() - last_time_upd > timeouts[screen])
        {
            last_time_upd = millis();
            screen = (screen + 1U) % 2U;
            showScreen();
        }
    }
    else
    {
        if(upd_ct_msg_flag)
        {
            upd_ct_msg_flag = 0U;
            showScreen();
        }
    }
}