#include "FastLED.h"
#include "globals.h"
#include "mathhandler.h"

// Add the number of channels because if you have 4 channels you need 4 "gaps" or spaces that dont exist
const int TotalLeds = NUM_LEDS + NUM_LEDS2 + NUM_LEDS3 + NUM_LEDS4 + NUM_CHANNELS;
CRGB leds[TotalLeds] = {CRGB::Blue};

int currentlyLitLedsForward[MAX_COMETS] = {0};
int currentlyLitLedsReverse[MAX_COMETS] = {0};

CRGB forwardColors[MAX_COMETS] = {CRGB::Blue};
CRGB reverseColors[MAX_COMETS] = {CRGB::Blue};

bool Strip2 = NUM_CHANNELS >= 2;
bool Strip3 = NUM_CHANNELS >= 3;
bool Strip4 = NUM_CHANNELS >= 4;

// The "Splitpoints" as I like to call them, establish a location in leds[] that doesn't exist on our strips, that way we can use these gaps as the endings for each strip
// EX: when the first strip comet hits LedSplit1 it will reset to the position of -1 (our reset point). Without these gaps the strip would show a lit LED randomly on the strip.

constexpr int LedSplit1 = NUM_LEDS + 1;
constexpr int LedSplit2 = LedSplit1 + NUM_LEDS2 + 1;
constexpr int LedSplit3 = LedSplit2 + NUM_LEDS3 + 1;


// These start points are based off the splits, this way if you change your strip sizes it'll still work fine.
constexpr int LedStart1 = 0;
constexpr int LedStart2 = LedSplit1 + 1;
constexpr int LedStart3 = LedSplit2 + 1;
constexpr int LedStart4 = LedSplit3 + 1;

// int specialSplitPoint = 15;

unsigned long previousMillis = 0;
const long interval = SPEED;

void fadeAll(); // Must be defined up here so it can be called before the actual definition

void initFastLED() {
  // According to FastLed documentation, we can do this a few ways, with multiple arrays, or with one main array with different offsets
  // I chose to use the offset method as I believe it will be better for this usage. (and it's easier)
  FastLED.addLeds<LED_TYPE, DATA_PIN1>(leds, LedStart1, NUM_LEDS);
  if (Strip2) {
    FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds, LedStart2, NUM_LEDS2);
    if (Strip3) {
    FastLED.addLeds<LED_TYPE, DATA_PIN3>(leds, LedStart3, NUM_LEDS3);
      if (Strip4) {
        FastLED.addLeds<LED_TYPE, DATA_PIN4>(leds, LedStart4, NUM_LEDS4);
      }
    }
  }
  FastLED.setBrightness(BRIGHTNESS);
}

void litArray() {
  unsigned long currentMillis = millis(); // millis() returns a time value, i believe its uptime since project started running

  if (currentMillis - previousMillis >= interval) { // if time now - last time checked (time passed) >= the delay interval, then run the rest (I'll refer to this as a frame)
    previousMillis = currentMillis; // set previous to current so we can check the time passed again in the next frame

    // Fade all LEDs, this is to give each LED a trail effect, and must be called each frame
    fadeToBlackBy(leds, TotalLeds, 255*0.33);

    // Handle the forward pulse
    for (int i = 0; i < MAX_COMETS; i++) { // creates a pulse as long as there aren't more than the maximum comets/pulses
      //strip 1
      if (currentlyLitLedsForward[i] >= 0 && currentlyLitLedsForward[i] <= TotalLeds) { // We make sure the pulse fits within the bounds of the strip (currentlyLitLedsForward[1] would return the position of the first comet in the array EX: 80 would be LED 80/100 if it's a 100 LED strip)
        leds[currentlyLitLedsForward[i]] = forwardColors[i];
        // Set the LED to the color passed to the func, leds[] is the array used to send data to a specific LED, to send color data to LED 100 we would use leds[100] = COLOR
        currentlyLitLedsForward[i]++; // Move the comet forward

        if (currentlyLitLedsForward[i] > TotalLeds || currentlyLitLedsForward[i] == LedSplit1 || currentlyLitLedsForward[i] == LedSplit2 || currentlyLitLedsForward[i] == LedSplit3) { // Check to see if it's gone to the end of the strip, then reset the comet
          currentlyLitLedsForward[i] = -1; // From what I understand, -1 does not appear on the strip unlike 0, so we use that as the "reset" point
        }
        // if (currentlyLitLedsForward[i] == specialSplitPoint) {
        //   if (randomZeroOrOne() == 1) {
        //     currentlyLitLedsForward[i] = 45;
        //   }
        // }
      }
    }

    // Handle the reverse pulse
    for (int i = 0; i < MAX_COMETS; i++) { // Does the same exact thing as forwardpulse function above, but in reverse
      if (currentlyLitLedsReverse[i] >= 0 && currentlyLitLedsReverse[i] <= TotalLeds) {    
        leds[currentlyLitLedsReverse[i]] = reverseColors[i];
        // Set the LED to the color passed to the func
        currentlyLitLedsReverse[i]--; // Move the comet backward

        if (currentlyLitLedsReverse[i] < 0 || currentlyLitLedsReverse[i] == LedSplit1 || currentlyLitLedsReverse[i] == LedSplit2 || currentlyLitLedsReverse[i] == LedSplit3) {
          // Reset the comet when it reaches the beginning
          currentlyLitLedsReverse[i] = -1; // We use the same reset point, although we could use NUM_LEDS + 1 too.
        }
      }
    }

    FastLED.show(); // This is what actually makes the data we defined above appear on the strip.
  }
}

void pulseEvent(CRGB color, int strip, int direction) { 
  bool reverse = (direction == 1) ? true : false; // reverse/in is 1, forward/out is 0
  int ledPosition;
  
  for (int i = 0; i < MAX_COMETS; i++) {
    if (!reverse) {
      ledPosition = (strip == 0) ? 0 : (strip == 1) ? LedSplit1 + 1 : (strip == 2) ? LedSplit2 + 1 : LedSplit3 + 1;
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = ledPosition;
        forwardColors[i] = color;
        break;
      }
    } else {
      ledPosition = (strip == 0) ? LedSplit1 - 1 : (strip == 1) ? LedSplit2 - 1 : (strip == 2) ? LedSplit3 - 1 : TotalLeds - 1;
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = ledPosition;
        reverseColors[i] = color;
        break;
      }
    }
  }
}
