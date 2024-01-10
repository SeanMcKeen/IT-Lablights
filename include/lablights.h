#include <FastLED.h>
#include <globals.h>

extern CRGB leds[NUM_LEDS];

void initFastLED();
void ledPulse(int pulseSize, CRGB pulseColor, int pulseSpeed);
void litArray();
void forwardEvent(CRGB fColor, int strip);
void reverseEvent(CRGB rColor, int strip);
void sendLightData(CRGB color, int pulses, int delayInterval, bool reverse);