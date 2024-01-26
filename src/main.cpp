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

unsigned long inPulseInterval = 0;
unsigned long inPulseInterval2 = 0;
unsigned long outPulseInterval = 0;
unsigned long outPulseInterval2 = 0;

//We need a previous pulse for each strips' forward and backward 2*NUM_CHANNELS
unsigned long previousPulseMillis = 0;
unsigned long previousPulseMillis2 = 0;
unsigned long previousPulseMillis3 = 0;
unsigned long previousPulseMillis4 = 0;
unsigned long previousPulseMillis5 = 0;
unsigned long previousPulseMillis6 = 0;
unsigned long previousPulseMillis7 = 0;
unsigned long previousPulseMillis8 = 0;

unsigned long pulsesToSendForward = 0;
unsigned long pulsesToSendForward2 = 0;
unsigned long pulsesToSendReverse = 0;
unsigned long pulsesToSendReverse2 = 0;

unsigned int pulsesSentForward = 0; // pulses sent forward on strip 1
unsigned int pulsesSentForward2 = 0; // pulses sent forward on strip 2 
unsigned int pulsesSentForward3 = 0;
unsigned int pulsesSentForward4 = 0;
unsigned int pulsesSentReverse = 0; // pulses sent reverse on strip 1 
unsigned int pulsesSentReverse2 = 0; // pulses sent reverse on strip 2 
unsigned int pulsesSentReverse3 = 0;
unsigned int pulsesSentReverse4 = 0;

CRGB forwardColor = CRGB::Blue;
CRGB forwardColor2 = CRGB::Blue;
CRGB reverseColor = CRGB::Blue;
CRGB reverseColor2 = CRGB::Blue;

int lastInAvg = 0;
int lastInAvg2 = 0;
int lastOutAvg = 0;
int lastOutAvg2 = 0;

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include <enablewifi.h>
#include <snmpgrab.h>
#include <lablights.h>
#include <mathhandler.h>
#endif

int Array1[] = ARRAY_1;

int ArrayTest[] = {1, 2};
int ArrayTest2[] = {3, 4};

// define functions
void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);

void setup() { // Main Setup
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    WifiBegin();
    SNMPsetup(ArrayTest, 2); // Must be called for every port array being used.
    SNMPsetup(ArrayTest2, 2); // Must be called for every port array being used.
    initFastLED();
  }
}

// Main loop
void loop() {
  if (PROJECT_NAME == "Lablights") {
    litArray();
    callLoop();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval; // this prevents drift in the delays

      printVariableHeader(); // Another func from snmp.cpp, prints debugging data in serial output

      // This block establishes the establishment of all variables for strip 1 pulses
      snmpLoop(ArrayTest, 2, 1); // snmpInLoop() can be found in snmp.cpp to see functionality
      int InAvg = arr1Totals[0];
      int OutAvg = arr1Totals[1];
      pulsesToSendReverse = calcSNMPPulses(InAvg); // The IN of the switch, we want the comet travelling from the end of the strip, back towards the device.
      reverseColor = calcPulseColor2(InAvg);
      inPulseInterval = pollTiming/pulsesToSendReverse * 1000; // We do this to spread the pulses evenly over 10 seconds, if we want to send 3 pulses it will send a pulse every 3333 milliseconds (3.3 seconds)
      pulsesToSendForward = calcSNMPPulses(OutAvg); // The OUT of the switch, we want the comet travelling from the beginning of the strip, away from the device.
      forwardColor = calcPulseColor(OutAvg);
      outPulseInterval = pollTiming/pulsesToSendForward * 1000;

      // This block establishes all variables for strip 2 pulses

      int InAvg2 = 0;
      int OutAvg2 = 0;
      if (Strip2){
        snmpLoop(ArrayTest2, 2, 2);
        InAvg2 = arr2Totals[0];
        OutAvg2 = arr2Totals[1];
        pulsesToSendReverse2 = calcSNMPPulses(InAvg2);
        reverseColor2 = calcPulseColor2(InAvg2);
        inPulseInterval2 = pollTiming/pulsesToSendReverse2 * 1000;
        pulsesToSendForward2 = calcSNMPPulses(OutAvg2);
        forwardColor2 = calcPulseColor(OutAvg2);
        outPulseInterval2 = pollTiming/pulsesToSendForward2 * 1000;
      }
      printVariableFooter();

      pulsesSentForward = 0; // Resetting these after each poll
      pulsesSentForward2 = 0;
      pulsesSentReverse = 0;
      pulsesSentReverse2 = 0;

      pulsesToSendForward2 = 1000;
      pulsesToSendForward = 1000;
    }
    // handle sending the pulses every x seconds
    // EACH pulse needs its own set of variables if you want it to be independent
    // Essentially we want each strip handling an array, since we have 4 strips, it would be appropriate to have 4 arrays.
    sendPulse(forwardColor, 1, &pulsesSentForward, pulsesToSendForward, &previousPulseMillis, outPulseInterval, 1);
    sendPulse(forwardColor2, 2, &pulsesSentForward2, pulsesToSendForward2, &previousPulseMillis2, outPulseInterval2, 1);

    sendPulse(reverseColor, 1, &pulsesSentReverse, pulsesToSendReverse, &previousPulseMillis3, inPulseInterval, 0);
    sendPulse(reverseColor2, 2, &pulsesSentReverse2, pulsesToSendReverse2, &previousPulseMillis4, inPulseInterval2, 0);
  }
}

// Lablights Functions
void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction){
  unsigned long currentMillis = millis();
  if (currentMillis - *prevMillisOfPulse >= interval){
    *prevMillisOfPulse = currentMillis;
    if (*pulsesSentVar < pulsesMaxSend){
      if (direction == 1){
        forwardEvent(color, strip);
      }else if (direction == 0){
        reverseEvent(color, strip);
      }
    }
  }
}