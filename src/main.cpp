#include <globals.h>
#include <secrets.h>
#include <mathhandler.h>
// Place header files above, headers are like pointers to other files in the project. 
// Headers must include function definitions for functions you want to use in other files. 
// Header files SHOULD be named the same as the cpp file they correspond to.

// Main Code Section -- Executed no matter what

// Defining local variables (These are set to 0 as they are redefined later in the code)
int pollInterval = POLL_DELAY;
unsigned long pollStart = 0;
unsigned long intervalBetweenPolls = 0;

unsigned long inPulseInterval = 0;
unsigned long outPulseInterval = 0;
unsigned long previousPulseMillis = 0;
unsigned long previousPulseMillis2 = 0;

unsigned long previousPulseMillis3 = 0;
unsigned long previousPulseMillis4 = 0;
unsigned long previousPulseMillis5 = 0;

unsigned long pulsesToSendForward = 0;
unsigned long pulsesToSendReverse = 0;
unsigned int pulsesSentForward = 0;
unsigned int pulsesSentReverse = 0;

CRGB forwardColor = CRGB::Blue;
CRGB reverseColor = CRGB::Blue;

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include <enablewifi.h>
#include <snmpgrab.h>
#include <lablights.h>
#endif

int Array1[] = ARRAY_1;

void setup() {
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    WifiBegin();
    SNMPsetup(Array1); // Must be called for every port array being used.
    initFastLED();
  }
}

// The "*" symbol tells it to use this variable to point to the one passed to the function, this allows me to update the original variable inside of this function.
// essentially this can be used as a time check that I wont have to replicate over and over and over
bool checkTimes(unsigned long *prevMillisToCheck, int IntervalToCheck) {
  unsigned long currentMillis = millis();
  if (currentMillis - *prevMillisToCheck >= IntervalToCheck){
    *prevMillisToCheck = currentMillis;
    return true;
  } else return false;
}

// again, just so I dont have to write it over and over and over...
bool checkPulses(int pulsesSent, int pulsesBeingSent){
  if (pulsesSent < pulsesBeingSent){
    return true;
  } else return false;
}

void loop() {
  litArray();
  if (PROJECT_NAME == "Lablights") {
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval; // this prevents drift in the delays
      int InAvg = snmpInLoop(Array1); // snmpInLoop() can be found in snmp.cpp to see functionality
      int OutAvg = snmpOutLoop(Array1); // snmpOutLoop() can be found in snmp.cpp to see functionality
      printVariableHeader(); // Another func from snmp.cpp, prints debugging data in serial output
      pulsesToSendReverse = calcSNMPPulses(InAvg); // The IN of the switch, we want the comet travelling from the end of the strip, back towards the device.
      reverseColor = calcPulseColor2(InAvg);
      inPulseInterval = 10/pulsesToSendReverse * 1000; // We do this to spread the pulses evenly over 10 seconds, if we want to send 3 pulses it will send a pulse every 3333 milliseconds (3.3 seconds)
      pulsesToSendForward = calcSNMPPulses(OutAvg); // The OUT of the switch, we want the comet travelling from the beginning of the strip, away from the device.
      forwardColor = calcPulseColor(OutAvg);
      outPulseInterval = 10/pulsesToSendForward * 1000;
      printVariableFooter();
      pulsesSentForward = 0; // Resetting these after each poll
      pulsesSentReverse = 0;
    }
    // handle sending the pulses every x seconds
    // the "&" symbol establishes that im going to be using it as a pointer (checkTimes() for more info)
    if (checkTimes(&previousPulseMillis, outPulseInterval)) {
      if (checkPulses(pulsesSentReverse, pulsesToSendReverse)){ // simple check to make sure we're sending the right amount
        pulsesSentReverse += 1;
        reverseEvent(reverseColor, 1); // func call to start a new pulse
        reverseEvent(reverseColor, 2);
        reverseEvent(reverseColor, 3);
        reverseEvent(reverseColor, 4);
      }
    }
    // same but forward instead of reverse
    if (checkTimes(&previousPulseMillis2, inPulseInterval)){
      if (checkPulses(pulsesSentForward, pulsesToSendForward)){
        pulsesSentForward += 1;
        forwardEvent(forwardColor, 1);
        forwardEvent(forwardColor, 2);
        forwardEvent(forwardColor, 3);
        forwardEvent(forwardColor, 4);
      }
    }
  }
}  