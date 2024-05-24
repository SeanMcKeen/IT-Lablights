#include "FastLED.h"
#include "globals.h"
#include "lablights.h"

const int totalLeds = NUM_LEDS + NUM_LEDS2 + NUM_LEDS3 + NUM_LEDS4 + NUM_CHANNELS;

CRGB leds[totalLeds];

// Initialize strips based on the number of channels
bool Strip2 = NUM_CHANNELS >= 2;
bool Strip3 = NUM_CHANNELS >= 3;
bool Strip4 = NUM_CHANNELS >= 4;

// The "Splitpoints" as I like to call them, establish a location in leds[] that doesn't exist on our strips, that way we can use these gaps as the endings for each strip
// EX: when the first strip comet hits LedSplit1 it will reset to the position of -1 (our reset point). Without these gaps the strip would show an unlit LED at the end of the strip.

int LedSplit1 = NUM_LEDS + 1;
int LedSplit2 = LedSplit1 + NUM_LEDS2 + 1;
int LedSplit3 = LedSplit2 + NUM_LEDS3 + 1;

// These start points are based off the splits, this way if you change your strip sizes it'll still work fine.
int LedStart2 = LedSplit1+1;
int LedStart3 = LedSplit2+1;
int LedStart4 = LedSplit3+1;

// func defs
int getStripSplit(int strip);
int getStripStart(int strip);

void setupFastLed() { // Setting FastLED logic to have all strips and offsets correctly
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
  if (Strip2) {
    FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds, LedStart2, NUM_LEDS2);
    if (Strip3) {
    FastLED.addLeds<LED_TYPE, DATA_PIN3>(leds, LedStart3, NUM_LEDS3);
      if (Strip4) {
        FastLED.addLeds<LED_TYPE, DATA_PIN4>(leds, LedStart4, NUM_LEDS4);
      }
    }
  }
  FastLED.setBrightness(255);
  // Will look like: <Strip LEDs> [GAP] <Strip2LEDs> [GAP] etc... for example lets say strip lengths of 30, 
  // if we have 2 strips the total amount will be 62 instead of 60. Strip 1 will be 0-29, gap will be 30, Strip 2 will be 31-61 and then another gap making it 62.
  // The gaps are necessary in order to define a pixel which is not VISIBLY on the strip, but still acts as a wall between them.
}

void CometManager::addComet(Comet comet) { // Place the comet on the correct strip (offsets may need to be changed)
  switch (comet.strip)
  {
  case 1:
    comet.position = 0-comet.length;
    break;
  case 2:
    comet.position = LedStart2-comet.length;
    break;
  case 3:
    comet.position = LedStart3-comet.length;
    break;
  case 4:
    comet.position = LedStart4-comet.length;
    break;
  default:
    break;
  }
  comets.push_back(comet);
}

void CometManager::removeComet(int index) {
  comets.erase(comets.begin() + index); // Erase the comet from the comets array
}

void CometManager::updateComets() { // Update the comets positions
  for (int i = 0; i < comets.size(); i++) { // For each comet
    int stripLength = getStripSplit(comets[i].strip);
    if (comets[i].position + comets[i].speed <= stripLength) { // If comet will be within bounds after the next movement
      comets[i].position = (comets[i].position + comets[i].speed); // set
    }

    if (comets[i].position >= stripLength) { // If the comet has gone out of bounds
      removeComet(i); // remove
    }
  }
}

void CometManager::renderComets() { // Draws the comets on the strip
  // Clear LEDs
  fill_solid(leds, totalLeds, CRGB::Black);
  
  // Render comets
  for (int i = 0; i < comets.size(); i++) { // for each comet in the comets array
    for (int j = 0; j < comets[i].length; j++) { // for each pixel in a comet
      int pos = (comets[i].position + j); // get the position of the pixel

      int thisStripSplit = getStripSplit(comets[i].strip);
      int thisStripStart = getStripStart(comets[i].strip);

      if (pos >= thisStripStart && pos < thisStripSplit) { // check if it's within bounds
        // Trail Fade/Color Logic
        int brightness = constrain(255 - (j*(255/comets[i].length)), 1, 245); // brightness/trail fade math
        leds[pos] = comets[i].color;
        leds[pos].fadeToBlackBy(brightness);
      }
    }
  }
  
  FastLED.show(); // show the changes
}

int getStripSplit(int strip) { // returns the "walls" or "gaps" I explained in the setupFastLed() function
  switch (strip)
  {
  case 1:
    return LedSplit1;
  case 2:
    return LedSplit2;
  case 3:
    return LedSplit3;
  default:
    return totalLeds;
  }
}

int getStripStart(int strip) {
  switch (strip)
  {
  case 1:
    return 0;
  case 2:
    return LedStart2;
  case 3:
    return LedStart3;
  default:
    return LedStart4;
  }
}