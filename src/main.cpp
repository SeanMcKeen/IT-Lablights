#include <Arduino.h>
#include "lablights.h"

CometManager cometManager;

int cometDelayMillis = 3000;
int prevMillis = 0;

void setup() {
  // Initialize FastLED
  setupFastLed();
}

void loop() {
  // Create a new comet and add it to the manager
  if (millis() > prevMillis + cometDelayMillis) {
    prevMillis = millis();
    
    Comet newComet(10, 1, CRGB::Red, 0);
    cometManager.addComet(newComet);
  }
  
  // Update and render comets
  cometManager.updateComets();
  cometManager.renderComets();
  
  // Delay for animation smoothness
}
