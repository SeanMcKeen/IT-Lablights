#include <globals.h>

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
#include "lablights.h"
#include "mathhandler.h"
#include "numsim.h"
#endif

int arrays[4][2];

void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);
void PrintVariables();

void setup() {
  if (PROJECT_NAME == "Lablights") {
    Serial.begin(115200);
    initFastLED();
    initSim();
  }
}

void loop() {
  if (PROJECT_NAME == "Lablights") {
    litArray();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval;
      simulateTraffic();
      for (int i = 0; i < 4; ++i) {
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
      if (DEBUG){PrintVariables();}
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
        pulseEvent(color, strip, 1);
      } else if (direction == 0) {
        pulseEvent(color, strip, 0);
      }
    }
  }
}

void PrintVariables(){
  for (int i = 1; i <= NUM_CHANNELS; i++){
    Serial.printf("Strip %i Averaged | IN: %i, OUT: %i\n", i, arrTotals[i-1][0], arrTotals[i-1][1]);
  }
  Serial.println("-------------------------");
}