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

// PulseInterval Variables (Must have a forward and reverse for each strip)
unsigned long inPulseInterval = 0;
unsigned long inPulseInterval2 = 0;
unsigned long inPulseInterval3 = 0;
unsigned long inPulseInterval4 = 0;

unsigned long outPulseInterval = 0;
unsigned long outPulseInterval2 = 0;
unsigned long outPulseInterval3 = 0;
unsigned long outPulseInterval4 = 0;

//We need a previous pulse for each strips' forward and backward, AKA 2*NUM_CHANNELS
unsigned long previousPulseMillis = 0;
unsigned long previousPulseMillis2 = 0;
unsigned long previousPulseMillis3 = 0;
unsigned long previousPulseMillis4 = 0;
unsigned long previousPulseMillis5 = 0;
unsigned long previousPulseMillis6 = 0;
unsigned long previousPulseMillis7 = 0;
unsigned long previousPulseMillis8 = 0;

// PulsesToSend Variables (Must have a forward and reverse for each strip)
unsigned long pulsesToSendForward = 0;
unsigned long pulsesToSendForward2 = 0;
unsigned long pulsesToSendForward3 = 0;
unsigned long pulsesToSendForward4 = 0;

unsigned long pulsesToSendReverse = 0;
unsigned long pulsesToSendReverse2 = 0;
unsigned long pulsesToSendReverse3 = 0;
unsigned long pulsesToSendReverse4 = 0;


// PulsesSent Variables (Must have a forward and reverse for each strip)
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
CRGB forwardColor3 = CRGB::Blue;
CRGB forwardColor4 = CRGB::Blue;

CRGB reverseColor = CRGB::Blue;
CRGB reverseColor2 = CRGB::Blue;
CRGB reverseColor3 = CRGB::Blue;
CRGB reverseColor4 = CRGB::Blue;

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
void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);

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
    litArray();
    callLoop();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval; // this prevents drift in the delays

      printVariableHeader(); // Another func from snmp.cpp, prints debugging data in serial output
      // This block establishes all variables for strip 1 pulses
      int InAvg; 
      int OutAvg;
      snmpLoop(Array1, 2, 1); // snmpLoop() can be found in snmp.cpp to see functionality
      InAvg = arr1Totals[0]; // arr1Totals is a global variable found in snmp.cpp and is set within the snmpLoop() function.
      OutAvg = arr1Totals[1];
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
        snmpLoop(Array2, 2, 2);
        InAvg2 = arr2Totals[0];
        OutAvg2 = arr2Totals[1];
        pulsesToSendReverse2 = calcSNMPPulses(InAvg2);
        reverseColor2 = calcPulseColor2(InAvg2);
        inPulseInterval2 = pollTiming/pulsesToSendReverse2 * 1000;
        pulsesToSendForward2 = calcSNMPPulses(OutAvg2);
        forwardColor2 = calcPulseColor(OutAvg2);
        outPulseInterval2 = pollTiming/pulsesToSendForward2 * 1000;
      }

      int InAvg3 = 0;
      int OutAvg3 = 0;
      if (Strip3){
        snmpLoop(Array3, 2, 3);
        InAvg3 = arr3Totals[0];
        OutAvg3 = arr3Totals[1];
        pulsesToSendReverse3 = calcSNMPPulses(InAvg3);
        reverseColor3 = calcPulseColor2(InAvg3);
        inPulseInterval3 = pollTiming/pulsesToSendReverse3 * 1000;
        pulsesToSendForward3 = calcSNMPPulses(OutAvg3);
        forwardColor3 = calcPulseColor(OutAvg3);
        outPulseInterval3 = pollTiming/pulsesToSendForward3 * 1000;
      }

      int InAvg4 = 0;
      int OutAvg4 = 0;
      if (Strip4){
        snmpLoop(Array4, 2, 4);
        InAvg4 = arr4Totals[0];
        OutAvg4 = arr4Totals[1];
        pulsesToSendReverse4 = calcSNMPPulses(InAvg4);
        reverseColor4 = calcPulseColor2(InAvg4);
        inPulseInterval4 = pollTiming/pulsesToSendReverse4 * 1000;
        pulsesToSendForward4 = calcSNMPPulses(OutAvg4);
        forwardColor4 = calcPulseColor(OutAvg4);
        outPulseInterval4 = pollTiming/pulsesToSendForward4 * 1000;
      }
      printVariableFooter();

      pulsesSentForward = 0; // Resetting these after each poll
      pulsesSentForward2 = 0;
      pulsesSentForward3 = 0;
      pulsesSentForward4 = 0;

      pulsesSentReverse = 0;
      pulsesSentReverse2 = 0;
      pulsesSentReverse3 = 0;
      pulsesSentReverse4 = 0;
    }
    // handle sending the pulses every x seconds
    // EACH pulse needs its own set of variables if you want it to be independent
    // Essentially we want each strip handling an array, since we have 4 strips, it would be appropriate to have 4 arrays.
    sendPulse(forwardColor, 1, &pulsesSentForward, pulsesToSendForward, &previousPulseMillis, outPulseInterval, 1); // call a pulse forward/OUT
    sendPulse(reverseColor, 1, &pulsesSentReverse, pulsesToSendReverse, &previousPulseMillis3, inPulseInterval, 0); // call a pulse reverse/IN

    sendPulse(forwardColor2, 2, &pulsesSentForward2, pulsesToSendForward2, &previousPulseMillis2, outPulseInterval2, 1);
    sendPulse(reverseColor2, 2, &pulsesSentReverse2, pulsesToSendReverse2, &previousPulseMillis4, inPulseInterval2, 0);

    sendPulse(forwardColor3, 3, &pulsesSentForward3, pulsesToSendForward3, &previousPulseMillis5, outPulseInterval3, 1);
    sendPulse(reverseColor3, 3, &pulsesSentReverse3, pulsesToSendReverse3, &previousPulseMillis6, inPulseInterval3, 0);

    sendPulse(forwardColor4, 4, &pulsesSentForward4, pulsesToSendForward4, &previousPulseMillis7, outPulseInterval4, 1);
    sendPulse(reverseColor4, 4, &pulsesSentReverse4, pulsesToSendReverse4, &previousPulseMillis8, inPulseInterval4, 0);
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