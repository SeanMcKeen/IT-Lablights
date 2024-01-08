#include <FastLED.h>
#include <globals.h>

extern CRGB leds[NUM_LEDS];

void initFastLED();
void ledPulse(int pulseSize, CRGB pulseColor, int pulseSpeed);
void litArray();
void forwardEvent(CRGB fColor);
void reverseEvent(CRGB rColor);
void sendLightData(CRGB color, int pulses, int delayInterval, bool reverse);