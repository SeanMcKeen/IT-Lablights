#include <Arduino.h>
#include "lablights.h"

CometManager cometManager;

int reDrawDelayMillis = 20;
int prevDraw = 0;

int cometDelayMillis = 1000;
int prevMillis = 0;

int prevMillis2 = 0;

void setup() {
  // Initialize FastLED
  setupFastLed();
  Serial.begin(115200);
}

void loop() {
  // Create a new comet and add it to the manager
  if (millis() - prevMillis >= cometDelayMillis) {
    prevMillis = millis();

    for (int i = 1; i < 5; i++) {
      Comet newComet(5, 1, CRGB::Red, i); // initialize a type of comets with a variable
      cometManager.addComet(newComet); // add it to the strip, logic in lablights.cpp
    }
    
  }
  // Update and render comets
  if (millis() - prevDraw >= reDrawDelayMillis) { // delay, without this it runs too fast but you can try it
    prevDraw = millis();
    cometManager.updateComets(); // updates position
    cometManager.renderComets(); // actually draws the comet
  }
}