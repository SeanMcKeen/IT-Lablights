#include "globals.h"
#include "vector"
#include "algorithm"
// Place header files above, headers are like pointers to other files in the project. 
// Headers must include function definitions for functions you want to use in other files. 
// Header files SHOULD be named the same as the cpp file they correspond to.

// Main Code Section -- Executed no matter what

// Here we define our globals.h variables into other variables.

// Defining local variables (These are set to 0 as they are redefined later in the code)
int pollInterval = POLL_DELAY;
int pollTiming = pollInterval/1000;
unsigned long prevPollMillis = 0;
unsigned long intervalBetweenPolls = 0;

const int channelAmount = NUM_CHANNELS;

// Thank you to the person in reddit for suggesting I use arrays!!
// PulseInterval Variables (Must have a forward and reverse for each strip)
unsigned long inPulseInterval[channelAmount] = {0};

unsigned long outPulseInterval[channelAmount] = {0};

//We need a previous pulse for each strips' forward and backward, AKA 2*NUM_CHANNELS
unsigned long previousPulseMillis[channelAmount*2] = {0};

// PulsesToSend Variables (Must have a forward and reverse for each strip)
int pulsesToSendForward[channelAmount] = {0};
int pulsesToSendReverse[channelAmount] = {0};

// PulsesSent Variables (Must have a forward and reverse for each strip)
int pulsesSentForward[channelAmount] = {0};
int pulsesSentReverse[channelAmount] = {0};

CRGB forwardColor[channelAmount] = {CRGB::Blue};
CRGB reverseColor[channelAmount] = {CRGB::Blue};

unsigned long ArrIns[channelAmount] = {0};
unsigned long ArrOuts[channelAmount] = {0};

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include "lablights.h"
#include "mathhandler.h"
#endif

// define functions
void sendPulses();
void clearData();

void setup() { // Main Setup
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    initFastLED();
  }
}

// Main loop
void loop() {
  if (PROJECT_NAME == "Lablights") {
    // callLoop();
    litArray();
    unsigned long currentMillis = millis();
    if (currentMillis - prevPollMillis >= pollInterval){

      if (DEBUG){
        size_t freeHeap = esp_get_free_heap_size();
        Serial.printf("Free heap size: %d bytes\n", freeHeap);
      }
      int index;

      for (int i = 0; i < channelAmount; i++){ // All variables will be index 0-3 unless holding/controlling ports
        srand((unsigned) time(NULL));
        ArrIns[index] = 1 + rand() % 1000000;
        ArrOuts[index] = 1 + rand() % 1000000;

        pulsesToSendForward[i] = calcSNMPPulses(ArrOuts[i]); // why does this crash on index 3????
        forwardColor[i] = calcPulseColor(ArrOuts[i]);

        pulsesToSendReverse[i] = calcSNMPPulses(ArrIns[i]);
        reverseColor[i] = calcPulseColor2(ArrIns[i]);
        outPulseInterval[i] = (pollTiming/pulsesToSendForward[i] * 1000);
        inPulseInterval[i] = (pollTiming/pulsesToSendReverse[i] * 1000);

        pulsesSentForward[i] = 0;
        pulsesSentReverse[i] = 0;

      }
      Serial.println("--------------------");
      Serial.printf("PORT %i IN: %i | OUT: %i\n",PORT1, ArrIns[0], ArrOuts[0]);
      Serial.printf("PORT %i IN: %i | OUT: %i\n",PORT2, ArrIns[1], ArrOuts[1]);
      Serial.printf("PORT %i IN: %i | OUT: %i\n",PORT3, ArrIns[2], ArrOuts[2]);
      Serial.printf("PORT %i IN: %i | OUT: %i\n",PORT4, ArrIns[3], ArrOuts[3]);
      Serial.println("--------------------");
      prevPollMillis = millis();
    }
    // handle sending the pulses every x seconds
    // EACH pulse needs its own set of variables if you want it to be independent
    // Essentially we want each strip handling an array, since we have 4 strips, it would be appropriate to have 4 arrays.
    sendPulses();
  }
}

// Lablights Functions
void sendPulse(int strip, int direction, int prev){
  unsigned long currentMillis = millis();
  int interval;
  CRGB color;
  if (direction == 0){
    interval = outPulseInterval[strip-1];
    color = forwardColor[strip-1];
  }else{
    interval = inPulseInterval[strip-1];
    color = reverseColor[strip-1];
  }

  if (currentMillis - previousPulseMillis[prev] >= interval){
    previousPulseMillis[prev] = currentMillis;
  if (direction == 0){
    if (pulsesSentForward[strip-1] < pulsesToSendForward[strip-1]){
      pulseEvent(color, strip, direction);
    }
  }else{
    if (pulsesSentReverse[strip-1] < pulsesToSendReverse[strip-1]){
      pulseEvent(color, strip, direction);
    }
  }
  }
}

void sendPulses() {
  for (int i = 0; i < 8; i++) {
    sendPulse(i / 2 + 1, i % 2, i);
  }
}