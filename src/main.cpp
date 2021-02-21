// Copyright 2021 jackw01. Released under the MIT License (see LICENSE for details).

#include <Arduino.h>
#include <FastLED.h>

#include "constants.hpp"
#include "fastledrgbw.hpp"

CRGB rgbLEDs[RGBLEDCount];
RGBW rgbwLEDs[RGBWLEDCount];
uint8_t * ledsRaw = (uint8_t *)rgbLEDs;

bool header = true;
uint32_t t = 0;
uint32_t lastByteTime = 0;
uint32_t lastAckTime = 0;
int16_t incomingByte;
uint8_t headerIndex = 0;
uint8_t checksum[2] = {};
uint16_t outIndex;
uint32_t bytesRemaining;

void clearAllLEDs() {
  for (uint8_t i = 0; i < RGBLEDCount; i++) rgbLEDs[i] = CRGB(0, 0, 0);
  for (uint8_t i = 0; i < RGBWLEDCount; i++) rgbwLEDs[i] = RGBW(0, 0, 0, 0);
}

void headerMode() {
  if (headerIndex < MagicWordSize){
    if (incomingByte == MagicWord[headerIndex]) headerIndex++;
    else headerIndex = 0;
  } else if (headerIndex >= MagicWordSize && headerIndex < MagicWordSize + 2) {
    checksum[headerIndex - MagicWordSize] = incomingByte;
    headerIndex++;
  } else if (headerIndex == MagicWordSize + 2) {
    if (incomingByte == (checksum[0] ^ checksum[1] ^ 0x55)) {
      bytesRemaining = 3L * (256L * (uint32_t)checksum[0] + (uint32_t)checksum[1] + 1L);
      outIndex = 0;
      clearAllLEDs();
      header = false; // Proceed to latch wait mode
    }
    headerIndex = 0; // Reset header position regardless of checksum result
  }
}

void dataMode() {
  if (outIndex < sizeof(rgbLEDs)) ledsRaw[outIndex++] = incomingByte;
  bytesRemaining--;

  if (bytesRemaining == 0) {
    for (uint8_t i = 0; i < RGBLEDCount && i < RGBWLEDCount; i++) {

      // RGB to RGBW conversion
      uint8_t r = rgbLEDs[i].r;
      uint8_t g = rgbLEDs[i].g;
      uint8_t b = rgbLEDs[i].b;
      uint8_t min = r < g ? (r < b ? r : b) : (g < b ? g : b);
      min = scale8(min, min);
      r -= min;
      g -= min;
      b -= min;
      rgbwLEDs[i] = RGBW(r, g, b, min);
    }

    FastLED.show();
    header = true;
    while (Serial.available() > 0) Serial.read(); // Flush serial buffer
  }
}

void ack() {
  Serial.print("Ada\n");
  lastAckTime = t;
}

void setup() {
  FastLED.addLeds<WS2812B, PinRGBLEDs, GRB>(rgbLEDs, RGBLEDCount);
  CRGB *rgbwLEDsRGB = (CRGB *) &rgbwLEDs[0]; // Hack to get RGBW to work
  FastLED.addLeds<WS2812B, PinRGBWLEDs>(rgbwLEDsRGB, getRGBWSize(RGBWLEDCount));
  FastLED.setBrightness(LEDBrightness);

  t = lastByteTime = lastAckTime = millis(); // Set initial counters

  Serial.begin(SerialSpeed);
  ack();
}

void loop() {
  t = millis();

  if ((incomingByte = Serial.read()) >= 0) {
    // New serial data?
    lastByteTime = lastAckTime = t;
    if (header) headerMode();
    else dataMode();
  } else {
    // Check for timeout
    if (t - lastAckTime > 1000) {
      ack();
      if (t - lastByteTime > SerialTimeout * 1000) {
        clearAllLEDs();
        header = true;
        lastByteTime = t;
      }
    }
  }
}
