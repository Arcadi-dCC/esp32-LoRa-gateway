#ifndef display_H
#define display_H

#include <platformTypes.h>

#include <Wire.h>

uint8 displayConfig(void);
void screenSequencer(void);
void showScreen(void);

extern uint8 screen;

#endif //display_H