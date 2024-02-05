#include <FastLED.h>
#include <globals.h>
#include <random>

// Add the number of channels because if you have 4 channels you need 4 "gaps" or spaces that dont exist
const int TotalLeds = NUM_LEDS + NUM_LEDS2 + NUM_LEDS3 + NUM_LEDS4 + NUM_CHANNELS;
CRGB leds[TotalLeds];

int currentlyLitLedsForward[TotalLeds];
int currentlyLitLedsReverse[TotalLeds];

CRGB forwardColors[TotalLeds];
CRGB reverseColors[TotalLeds];

double fadeForwardAmount[TotalLeds];
double fadeReverseAmount[TotalLeds];

int forwardPulseLengths[TotalLeds];
int reversePulseLengths[TotalLeds];

#if NUM_CHANNELS == 2
  bool Strip2 = true;
  bool Strip3 = false;
  bool Strip4 = false;
#elif NUM_CHANNELS == 3
  bool Strip2 = true;
  bool Strip3 = true;
  bool Strip4 = false;
#elif NUM_CHANNELS == 4
  bool Strip2 = true;
  bool Strip3 = true;
  bool Strip4 = true;
#endif

// The "Splitpoints" as I like to call them, establish a location in leds[] that doesn't exist on our strips, that way we can use these gaps as the endings for each strip
// EX: when the first strip comet hits LedSplit1 it will reset to the position of -1 (our reset point). Without these gaps the strip would show an unlit LED at the end of the strip.

int LedSplit1 = NUM_LEDS + 1;
int LedSplit2 = LedSplit1 + NUM_LEDS2 + 1;
int LedSplit3 = LedSplit2 + NUM_LEDS3 + 1;

// These start points are based off the splits, this way if you change your strip sizes it'll still work fine.
int LedStart1 = 0;
int LedStart2 = LedSplit1+1;
int LedStart3 = LedSplit2+1;
int LedStart4 = LedSplit3+1;

unsigned long previousMillis = 0;
const long interval = SPEED;

void fadePulse(int comet, int trailLength, bool reverse); // Must be defined up here so it can be called before the actual definition

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
    // fill_solid(leds, TotalLeds, CRGB::Black);
    // Handle the forward pulse
    for (int i = 0; i < TotalLeds; i++) { // creates a pulse as long as there aren't more than the maximum comets/pulses 
      if (currentlyLitLedsForward[i] >= 0 && currentlyLitLedsForward[i] <= TotalLeds) { // We make sure the pulse fits within the bounds of the strip (currentlyLitLedsForward[1] would return the position of the first comet in the array EX: 80 would be LED 80/100 if it's a 100 LED strip)
        leds[currentlyLitLedsForward[i]] += forwardColors[i];

        fadePulse(currentlyLitLedsForward[i], forwardPulseLengths[i], false);
        // Set the LED to the color passed to the func, leds[] is the array used to send data to a specific LED, to send color data to LED 100 we would use leds[100] = COLOR
        currentlyLitLedsForward[i]++; // Move the comet forward

        if (currentlyLitLedsForward[i] > TotalLeds || currentlyLitLedsForward[i] == LedSplit1 || currentlyLitLedsForward[i] == LedSplit2 || currentlyLitLedsForward[i] == LedSplit3) { // Check to see if it's gone to the end of the strip, then reset the comet
          currentlyLitLedsForward[i] = -1; // From what I understand, -1 does not appear on the strip unlike 0, so we use that as the "reset" point
        }
      }
    }

    // Handle the reverse pulse
    for (int i = 0; i < TotalLeds; i++) { // Does the same exact thing as forwardpulse function above, but in reverse
      if (currentlyLitLedsReverse[i] >= 0 && currentlyLitLedsReverse[i] <= TotalLeds) {    
        leds[currentlyLitLedsReverse[i]] += reverseColors[i];

        fadePulse(currentlyLitLedsReverse[i], reversePulseLengths[i], true);
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

void fadePulse(int comet, int trailLength, bool reverse) {
  trailLength = 5;
  CRGB originColor = leds[comet];
  double fadeAmount = 0.2;  // Adjust the fade amount as needed

  if (!reverse) {
    for (int i = 1; i <= trailLength; i++) {
      double fadeFraction = (double)i / trailLength;
      leds[comet - i] = originColor.fadeToBlackBy(fadeFraction * 255);
    }
  } else {
    for (int i = 1; i <= trailLength; i++) {
      double fadeFraction = (double)i / trailLength;
      leds[comet - i] = originColor.fadeToBlackBy(fadeFraction * 255);
    }
  }
}


void forwardEvent(CRGB color, int strip) { // This can be called to start a new pulse originating from the beginning of the strip, with the desired color
  // Find the first available position in currentlyLitLedsForward and set it to 0
  for (int i = 0; i < TotalLeds; i++) {
    // Find the first available comet that's ready to be shot.
    if (strip == 1) {
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = 0; // We set it to 0 as our logic makes it so anything within the range of the strip will constantly be moved. -1 is not within the range so it is ignored until redefined here.
        
        forwardColors[i] = color; // This is how we save what comets have what color values.
        forwardPulseLengths[i] = 5;
        fadeForwardAmount[i] = 0.2;
        break;
      }
    }else if (strip == 2){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit1 + 1;

        forwardColors[i] = color; // This is how we save what comets have what color values.
        forwardPulseLengths[i] = 5;
        fadeForwardAmount[i] = 0.2;
        break;
      }
    }else if (strip == 3){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit2 + 1;

        forwardColors[i] = color; // This is how we save what comets have what color values.
        forwardPulseLengths[i] = 5;
        fadeForwardAmount[i] = 0.2;
        break;
      }
    }else if (strip == 4){
      if (currentlyLitLedsForward[i] == -1) {
        currentlyLitLedsForward[i] = LedSplit3 + 1;

        forwardColors[i] = color; // This is how we save what comets have what color values.
        forwardPulseLengths[i] = 5;
        fadeForwardAmount[i] = 0.2;
        break;
      }
    }
  }
}

void reverseEvent(CRGB color, int strip) { // This is the exact same as forwardEvent() but in reverse.
  // Find the first available position in currentlyLitLedsReverse and set it to NUM_LEDS - 1
  for (int i = 0; i < TotalLeds; i++) {
    if (strip == 1) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit1 - 1;

        reverseColors[i] = color;
        reversePulseLengths[i] = 2;
        fadeReverseAmount[i] = 0.5;
        break;
      }
    }else if (strip == 2) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit2 - 1;

        reverseColors[i] = color;
        reversePulseLengths[i] = 2;
        fadeReverseAmount[i] = 0.5;
        break;
      }
    }else if (strip == 3) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = LedSplit3 - 1;

        reverseColors[i] = color;
        reversePulseLengths[i] = 2;
        fadeReverseAmount[i] = 0.5;
        break;
      }
    }else if (strip == 4) {
      if (currentlyLitLedsReverse[i] == -1) {
        currentlyLitLedsReverse[i] = TotalLeds - 1;

        reverseColors[i] = color;
        reversePulseLengths[i] = 2;
        fadeReverseAmount[i] = 0.5;
        break;
      }
    }
  }
}