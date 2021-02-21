// Copyright 2020 jackw01. Released under the MIT License (see LICENSE for details).

#include "fastledrgbw.hpp"

// Create the CRGB array with extra LEDs to account for the 33% increase in data needed
uint16_t getRGBWSize(uint16_t numLEDs){
  uint16_t bytes = numLEDs * 4;
  if(bytes % 3 > 0) return bytes / 3 + 1;
  else return bytes / 3;
}
