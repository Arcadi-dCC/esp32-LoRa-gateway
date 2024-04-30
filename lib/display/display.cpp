#include <Arduino.h>

#include <display.h>
#include <displayCfg.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>                                       

float64 distance = 0U;
uint16 next_bins[6] = {0U, 0U, 0U, 0U, 0U, 0U};