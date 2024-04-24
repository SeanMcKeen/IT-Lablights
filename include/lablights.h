#include <FastLED.h>
#include <globals.h>

extern CRGB leds[NUM_LEDS];

extern bool Strip1;
extern bool Strip2;
extern bool Strip3;
extern bool Strip4;

void initFastLED();
void ledPulse(int pulseSize, CRGB pulseColor, int pulseSpeed);
void litArray();
void forwardEvent(CRGB fColor, int strip);
void reverseEvent(CRGB rColor, int strip);
void sendLightData(CRGB color, int pulses, int delayInterval, bool reverse);