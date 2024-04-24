#include "FastLED.h"
#include "globals.h"
#include "lablights.h"

CRGB leds[NUM_LEDS];

void setupFastLed() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void CometManager::addComet(Comet comet) {
  comets.push_back(comet);
}

void CometManager::removeComet(int index) {
  comets.erase(comets.begin() + index);
}

void CometManager::updateComets() {
  for (int i = 0; i < comets.size(); i++) {
    comets[i].position = (comets[i].position + comets[i].speed) % NUM_LEDS;
  }
}

void CometManager::renderComets() {
  // Clear LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Render comets
  for (int i = 0; i < comets.size(); i++) {
    for (int j = 0; j < comets[i].length; j++) {
      int pos = (comets[i].position + j) % NUM_LEDS;
      leds[pos] = comets[i].color;
    }
  }
  
  FastLED.show();
}
