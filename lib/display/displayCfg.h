#ifndef displayCfg_H
#define displayCfg_H

#define DSP_WIDTH 16U //display width, in pixels
#define DSP_HEIGHT 2U //display hieght, in pixels

//#define DSP_ADDR 0x3C //display I2C address       //OLED-only
//#define DSP_RESET -1 //reset pin. No reset = -1   //OLED-only

#define MSG_0 "Distance:"
#define MSG_0_TOUT 5000U //timeout for message 0 [ms]

#define MSG_1 "Collect bins:"
#define MSG_1_TOUT 10000U //timeout for message 1 [ms]

#define MSG_2 "Follow the route"
#define MSG_2_TOUT 0U //timeout for message 2 [ms]. 0 means no timeout.

#define MSG_3 "All bins empty."
#define MSG_3_COMP "Return to base."
#define MSG_3_TOUT 0U //timeout for message 3 [ms]. 0 means no timeout.

#endif //displayCfg_H