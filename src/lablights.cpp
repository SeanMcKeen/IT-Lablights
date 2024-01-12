#include <FastLED.h>
#include <globals.h>

// Here we handle variable creation depending on channels (add more if more channels)
CRGB leds[NUM_LEDS + NUM_LEDS2 + NUM_LEDS3 + NUM_LEDS4];

int currentlyLitLedsForward[MAX_COMETS];
int currentlyLitLedsReverse[MAX_COMETS];

CRGB forwardColors[MAX_COMETS];
CRGB reverseColors[MAX_COMETS];

int LedSplit1 = NUM_LEDS;
int LedSplit2 = LedSplit1 + NUM_LEDS2;
int LedSplit3 = LedSplit2 + NUM_LEDS3;
int TotalLeds = LedSplit3 + NUM_LEDS4;

unsigned long previousMillis = 0;
const long interval = 20; // Interval in milliseconds (How fast the pulses travel, decreasing this value increases the speeds)

void fadeAll(); // Must be defined up here so it can be called before the actual definition

void initFastLED() {
  FastLED.addLeds<LED_TYPE, DATA_PIN1>(leds, 0, NUM_LEDS);
  if (NUM_CHANNELS == 2) {
    FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds, LedSplit1, NUM_LEDS2);

  }else if (NUM_CHANNELS == 3) {
    FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds, LedSplit1, NUM_LEDS2);
    FastLED.addLeds<LED_TYPE, DATA_PIN3>(leds, LedSplit2, NUM_LEDS3);

  }else if (NUM_CHANNELS == 4) {
    FastLED.addLeds<LED_TYPE, DATA_PIN2>(leds, LedSplit1, NUM_LEDS2);
    FastLED.addLeds<LED_TYPE, DATA_PIN3>(leds, LedSplit2, NUM_LEDS3);
    FastLED.addLeds<LED_TYPE, DATA_PIN4>(leds, LedSplit3, NUM_LEDS4);

  }
  FastLED.setBrightness(BRIGHTNESS);
}

void litArray() {
  unsigned long currentMillis = millis(); // millis() returns a time value, i believe its uptime since project started running

  if (currentMillis - previousMillis >= interval) { // if time now - last time checked (time passed) >= the delay interval, then run the rest (I'll refer to this as a frame)
    previousMillis = currentMillis; // set previous to current so we can check the time passed again in the next frame

    // Fade all LEDs, this is to give each LED a trail effect, and must be called each frame
    fadeAll();

    // Handle the forward pulse
    for (int i = 0; i < MAX_COMETS; i++) { // creates a pulse as long as there aren't more than the maximum comets/pulses
      //strip 1
      if (currentlyLitLedsForward[i] >= 0 && currentlyLitLedsForward[i] <= TotalLeds) { // We make sure the pulse fits within the bounds of the strip (currentlyLitLedsForward[1] would return the position of the first comet in the array EX: 80 would be LED 80/100 if it's a 100 LED strip)
        leds[currentlyLitLedsForward[i]] = forwardColors[i]; // Set the LED to the color passed to the func, leds[] is the array used to send data to a specific LED, to send color data to LED 100 we would use leds[100] = COLOR
        currentlyLitLedsForward[i]++; // Move the comet forward

        if (currentlyLitLedsForward[i] > TotalLeds || currentlyLitLedsForward[i] == LedSplit1 || currentlyLitLedsForward[i] == LedSplit2 || currentlyLitLedsForward[i] == LedSplit3) { // Check to see if it's gone to the end of the strip, then reset the comet
          currentlyLitLedsForward[i] = -1; // From what I understand, -1 does not appear on the strip unlike 0, so we use that as the "reset" point
        }
      }
    }

    // Handle the reverse pulse
    for (int i = 0; i < MAX_COMETS; i++) { // Does the same exact thing as forwardpulse function above, but in reverse
      if (currentlyLitLedsReverse[i] >= 0 && currentlyLitLedsReverse[i] <= TotalLeds) {
        leds[currentlyLitLedsReverse[i]] = reverseColors[i]; // Set the LED to the color passed to the func
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

void fadeAll() {
  // Fade out all LEDs
  for (int i = 0; i < TotalLeds; i++) { // We call fade out BEFORE establishing the pulse so that only the trail is being faded and not the head of the pulse.
  // Fades the color by an equal amount, that way the color appears the same while becomming dimmer. Note: do NOT use brightness to accomplish this as brightness is global to the entire strip and cannot be used for a single pulse.
    if (leds[i].r > 0) leds[i].r -= leds[i].r*.20;
    if (leds[i].g > 0) leds[i].g -= leds[i].g*.20;
    if (leds[i].b > 0) leds[i].b -= leds[i].b*.20;
  }
}

void forwardEvent(CRGB color, int strip) { // This can be called to start a new pulse originating from the beginning of the strip, with the desired color
  // Find the first available position in currentlyLitLedsForward and set it to 0
  for (int i = 0; i < MAX_COMETS; i++) {
    // Find the first available comet that's ready to be shot.
    if (strip == 1) {
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = 0; // We set it to 0 as our logic makes it so anything within the range of the strip will constantly be moved. -1 is not within the range so it is ignored until redefined here.

        forwardColors[i] = color; // This is how we save what comets have what color values.
        break;
      }
    }else if (strip == 2){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit1 + 1;

        forwardColors[i] = color;
        break;
      }
    }else if (strip == 3){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit2 + 1;

        forwardColors[i] = color;
        break;
      }
    }else if (strip == 4){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit3 + 1;

        forwardColors[i] = color;
        break;
      }
    }
  }
}

void reverseEvent(CRGB color, int strip) { // This is the exact same as forwardEvent() but in reverse.
  // Find the first available position in currentlyLitLedsReverse and set it to NUM_LEDS - 1
  for (int i = 0; i < MAX_COMETS; i++) {
    if (strip == 1) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit1 - 1;

        reverseColors[i] = color;
        break;
      }
    }else if (strip == 2) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit2 - 1;

        reverseColors[i] = color;
        break;
      }
    }else if (strip == 3) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit3 - 1;

        reverseColors[i] = color;
        break;
      }
    }else if (strip == 4) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = TotalLeds - 1;

        reverseColors[i] = color;
        break;
      }
    }
  }
}