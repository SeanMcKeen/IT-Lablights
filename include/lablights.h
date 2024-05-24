#ifndef LABLIGHTS_H
#define LABLIGHTS_H

#include <vector>
#include <FastLED.h>
#include "globals.h"

class Comet {
public:
  int length;
  int speed;
  CRGB color;
  int position;
  int strip;

  Comet(int length, int speed, CRGB color, int strip) {
    this->length = length;
    this->speed = speed;
    this->color = color;
    this->strip = strip;
  }
};

class CometManager {
public:
  std::vector<Comet> comets;

  void addComet(Comet comet);
  void removeComet(int index);
  void updateComets();
  void renderComets();
};

extern CometManager cometManager;

void setupFastLed();

#endif