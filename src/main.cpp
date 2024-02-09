#include <globals.h>
#include <secrets.h>
// Place header files above, headers are like pointers to other files in the project. 
// Headers must include function definitions for functions you want to use in other files. 
// Header files SHOULD be named the same as the cpp file they correspond to.

// Main Code Section -- Executed no matter what

// Defining local variables (These are set to 0 as they are redefined later in the code)
int pollInterval = POLL_DELAY;
int pollTiming = pollInterval/1000;
unsigned long pollStart = 0;
unsigned long intervalBetweenPolls = 0;

// Thank you to the person in reddit for suggesting I use arrays!!
// PulseInterval Variables (Must have a forward and reverse for each strip)
unsigned long inPulseInterval[NUM_CHANNELS] = {};

unsigned long outPulseInterval[NUM_CHANNELS] = {};

//We need a previous pulse for each strips' forward and backward, AKA 2*NUM_CHANNELS
unsigned long previousPulseMillis[NUM_CHANNELS*2] = {};

// PulsesToSend Variables (Must have a forward and reverse for each strip)
int pulsesToSendForward[NUM_CHANNELS] = {};
int pulsesToSendReverse[NUM_CHANNELS] = {};

// PulsesSent Variables (Must have a forward and reverse for each strip)
int pulsesSentForward[NUM_CHANNELS] = {};
int pulsesSentReverse[NUM_CHANNELS] = {};

CRGB forwardColor[] = {CRGB::Blue};
CRGB reverseColor[] = {CRGB::Blue};

int InAvg[NUM_CHANNELS] = {};
int OutAvg[NUM_CHANNELS] = {};

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include <enablewifi.h>
#include <snmpgrab.h>
#include <lablights.h>
#include <mathhandler.h>
#endif

// Here we define our globals.h variables into other variables.
int Array1[] = ARRAY_1;
int Array2[] = ARRAY_2;
int Array3[] = ARRAY_3;
int Array4[] = ARRAY_4;

// define functions
void sendPulse(CRGB color, int strip, int *PulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);

void setup() { // Main Setup
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    WifiBegin();
    SNMPsetup(Array1, 2); // Must be called for every port array being used.
    if (Strip2){SNMPsetup(Array2, 2);}
    if (Strip3){SNMPsetup(Array3, 2);}
    if (Strip4){SNMPsetup(Array4, 2);}
    initFastLED();
  }
}

// Main loop
void loop() {
  if (PROJECT_NAME == "Lablights") {
    callLoop();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval; // this prevents drift in the delays

      printVariableHeader(); // Another func from snmp.cpp, prints debugging data in serial output
      // This block establishes all variables for strip 1 pulses
      for (int x = 0; x < NUM_CHANNELS; x++){
        Serial.println("loop");
        switch (x)
        {
        case 0:
          snmpLoop(Array1, sizeof(Array1), x);
          break;
        }
        // InAvg[x] = arrINTotals[x];
        // OutAvg[x] = arrOUTTotals[x];
        // pulsesToSendForward[x] = calcSNMPPulses(OutAvg[x]);
        // forwardColor[x] = calcPulseColor(OutAvg[x]);
        // outPulseInterval[x] = (pollTiming/pulsesToSendForward[x] * 1000);

        // pulsesToSendReverse[x] = calcSNMPPulses(InAvg[x]);
        // reverseColor[x] = calcPulseColor2(InAvg[x]);
        // inPulseInterval[x] = (pollTiming/pulsesToSendReverse[x] * 1000);

        // pulsesSentForward[x] = 0;
        // pulsesSentReverse[x] = 0;
      }
      printVariableFooter();
    }
    // handle sending the pulses every x seconds
    // EACH pulse needs its own set of variables if you want it to be independent
    // Essentially we want each strip handling an array, since we have 4 strips, it would be appropriate to have 4 arrays.
    sendPulse(forwardColor[0], 1, &pulsesSentForward[0], pulsesToSendForward[0], &previousPulseMillis[0], outPulseInterval[0], 0); // call a pulse forward/OUT
    sendPulse(reverseColor[0], 1, &pulsesSentReverse[0], pulsesToSendReverse[0], &previousPulseMillis[1], inPulseInterval[0], 1); // call a pulse reverse/IN

    sendPulse(forwardColor[1], 2, &pulsesSentForward[1], pulsesToSendForward[1], &previousPulseMillis[2], outPulseInterval[1], 0);
    sendPulse(reverseColor[1], 2, &pulsesSentReverse[1], pulsesToSendReverse[1], &previousPulseMillis[3], inPulseInterval[1], 1);

    sendPulse(forwardColor[2], 3, &pulsesSentForward[2], pulsesToSendForward[2], &previousPulseMillis[4], outPulseInterval[2], 0);
    sendPulse(reverseColor[2], 3, &pulsesSentReverse[2], pulsesToSendReverse[2], &previousPulseMillis[5], inPulseInterval[2], 1);

    sendPulse(forwardColor[3], 4, &pulsesSentForward[3], pulsesToSendForward[3], &previousPulseMillis[6], outPulseInterval[3], 0);
    sendPulse(reverseColor[3], 4, &pulsesSentReverse[3], pulsesToSendReverse[3], &previousPulseMillis[7], inPulseInterval[3], 1);
    litArray();
  }
}

// Lablights Functions
void sendPulse(CRGB color, int strip, int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction){
  unsigned long currentMillis = millis();
  if (currentMillis - *prevMillisOfPulse >= interval){
    *prevMillisOfPulse = currentMillis;
    if (*pulsesSentVar < pulsesMaxSend){
      pulseEvent(color, strip, direction);
    }
  }
}