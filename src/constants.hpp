// Copyright 2021 jackw01. Released under the MIT License (see LICENSE for details).

#pragma once

#include "fastledrgbw.hpp"

const uint8_t LEDBrightness = 255;

// For RGB LEDs
const uint8_t PinRGBLEDs = 6;
const uint8_t RGBLEDCount = 34;
const bool CorrectRGB = false; // correction below only applied if true
const CRGB RGBLEDCorrection = CRGB(255, 178, 178);

// For RGBW LEDs
const uint8_t PinRGBWLEDs = 13;
const uint8_t RGBWLEDCount = 150;
const bool CorrectRGBW = false; // correction below only applied if true
const RGBW RGBWLEDCorrection = RGBW(255, 220, 255, 255);

const uint32_t SerialSpeed = 115200;
const uint16_t SerialTimeout = 150; // Seconds

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
