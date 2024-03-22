#include "globals.h"
#include <secrets.h>

// Define global variables
int pollInterval = POLL_DELAY;
int pollTiming = pollInterval / 1000;
unsigned long pollStart = 0;
unsigned long intervalBetweenPolls = 0;
unsigned long inPulseInterval[4] = {0};
unsigned long outPulseInterval[4] = {0};
unsigned long previousPulseMillis[8] = {0};
unsigned long pulsesToSendForward[4] = {0};
unsigned long pulsesToSendReverse[4] = {0};
unsigned int pulsesSentForward[4] = {0};
unsigned int pulsesSentReverse[4] = {0};
CRGB forwardColor[4] = {CRGB::Blue};
CRGB reverseColor[4] = {CRGB::Blue};

#if LABLIGHTS
#include <enablewifi.h>
#include <snmpgrab.h>
#include <lablights.h>
#include <mathhandler.h>
#endif

int arrays[4][2] = {ARRAY_1, ARRAY_2, ARRAY_3, ARRAY_4};

void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);

void setup() {
  if (PROJECT_NAME == "Lablights") {
    Serial.begin(115200);
    WifiBegin();
    for (int i = 0; i < 4; ++i) {
      if (i >= Strip2 && arrays[i]) {
        SNMPsetup(arrays[i], 2);
      }
    }
    initFastLED();
  }
}

void loop() {
  if (PROJECT_NAME == "Lablights") {
    litArray();
    callLoop();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval;
      if (SNMPDEBUG) {printVariableHeader();}
      for (int i = 0; i < 4; ++i) {
        if (arrays[i]) {
          snmpLoop(arrays[i], 2, i + 1);
          int InAvg = arrTotals[i][0];
          int OutAvg = arrTotals[i][1];
          pulsesToSendReverse[i] = calcSNMPPulses(InAvg);
          reverseColor[i] = calcPulseColor2(InAvg);
          inPulseInterval[i] = pollTiming / pulsesToSendReverse[i] * 1000;
          pulsesToSendForward[i] = calcSNMPPulses(OutAvg);
          forwardColor[i] = calcPulseColor(OutAvg);
          outPulseInterval[i] = pollTiming / pulsesToSendForward[i] * 1000;
          pulsesSentForward[i] = 0;
          pulsesSentReverse[i] = 0;
        }
      }
      if (SNMPDEBUG) {printVariableFooter();}
    }
    for (int i = 0; i < 4; ++i) {
      if (arrays[i]) {
        sendPulse(forwardColor[i], i + 1, &pulsesSentForward[i], pulsesToSendForward[i], &previousPulseMillis[i*2], outPulseInterval[i], 1);
        sendPulse(reverseColor[i], i + 1, &pulsesSentReverse[i], pulsesToSendReverse[i], &previousPulseMillis[i*2+1], inPulseInterval[i], 0);
      }
    }
  }
}

void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction) {
  unsigned long currentMillis = millis();
  if (currentMillis - *prevMillisOfPulse >= interval) {
    *prevMillisOfPulse = currentMillis;
    if (*pulsesSentVar < pulsesMaxSend) {
      if (direction == 1) {
        forwardEvent(color, strip);
      } else if (direction == 0) {
        reverseEvent(color, strip);
      }
    }
  }
}
