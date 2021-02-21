// Copyright 2021 jackw01. Released under the MIT License (see LICENSE for details).

#include <Arduino.h>
#include <FastLED.h>

const uint8_t PinLEDs = 6;
const uint8_t LEDCount = 26;
const uint8_t LEDBrightness = 255;

const uint32_t SerialSpeed = 115200;
const uint16_t SerialTimeout = 150; // Seconds

CRGB leds[LEDCount];
uint8_t * ledsRaw = (uint8_t *)leds;

// A 'magic word' (along with LED count & checksum) precedes each block
// of LED data; this assists the microcontroller in syncing up with the
// host-side software and properly issuing the latch (host I/O is
// likely buffered, making usleep() unreliable for latch). You may see
// an initial glitchy frame or two until the two come into alignment.
// The magic word can be whatever sequence you like, but each character
// should be unique, and frequent pixel values like 0 and 255 are
// avoided -- fewer false positives. The host software will need to
// generate a compatible header: immediately following the magic word
// are three bytes: a 16-bit count of the number of LEDs (high byte
// first) followed by a simple checksum value (high byte XOR low byte
// XOR 0x55). LED data follows, 3 bytes per LED, in order R, G, B,
// where 0 = off and 255 = max brightness.

const uint8_t MagicWord[] = {'A', 'd', 'a'};
const uint8_t MagicWordSize = 3;

bool header = true;
uint32_t t = 0;
uint32_t lastByteTime = 0;
uint32_t lastAckTime = 0;
int16_t incomingByte;
uint8_t headerIndex = 0;
uint8_t checksum[2] = {};
uint16_t outIndex;
uint32_t bytesRemaining;

void setAllLEDs(CRGB color) {
  for (uint8_t i = 0; i < LEDCount; i++) leds[i] = color;
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
      setAllLEDs(CRGB(0, 0, 0));
      header = false; // Proceed to latch wait mode
    }
    headerIndex = 0; // Reset header position regardless of checksum result
  }
}

void dataMode() {
  if (outIndex < sizeof(leds)) ledsRaw[outIndex++] = incomingByte;
  bytesRemaining--;

  if (bytesRemaining == 0) {
    header = true;
    FastLED.show();
    while (Serial.available() > 0) Serial.read(); // Flush serial buffer
  }
}

void ack() {
  Serial.print("Ada\n");
  lastAckTime = t;
}

void setup() {
  FastLED.addLeds<WS2812B, PinLEDs, GRB>(leds, LEDCount);
  FastLED.setCorrection(TypicalLEDStrip);
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
        setAllLEDs(CRGB(0, 0, 0));
        header = true;
        lastByteTime = t;
      }
    }
  }
}
