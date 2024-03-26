#include <globals.h>
#include <secrets.h>
#include <vector>
#include <iostream>
#include <esp_log.h>


#if LABLIGHTS
#include "enablewifi.h"
#include "snmpgrab.h"
#include "lablights.h"
#include "mathhandler.h"
#include "cloudserver.h"
#include <FastLED.h>
#endif

// Define global variables
int pollInterval = POLL_DELAY;
int pollTiming = pollInterval / 1000;
unsigned long pollStart = 0;
unsigned long inPulseInterval[4] = {0};
unsigned long outPulseInterval[4] = {0};
unsigned long previousPulseMillis[8] = {0};
unsigned int pulsesToSendForward[4] = {1};
unsigned int pulsesToSendReverse[4] = {1};
unsigned int pulsesSentForward[4] = {0};
unsigned int pulsesSentReverse[4] = {0};
CRGB forwardColor[4] = {CRGB::Blue};
CRGB reverseColor[4] = {CRGB::Blue};

int blinkDelay = 500;
unsigned long prevBlink = 0;

bool snmpLoopFinished = false; // Declare a flag variable

std::vector<int> arrays;

void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);
void vectorInsert();

void setup() {
  pinMode(PIN_BL, OUTPUT);
  if (PROJECT_NAME == "Lablights") {
    Serial.begin(115200);
    WifiBegin();
    vectorInsert();
    SNMPsetup(arrays);
    initFastLED();
    webServSetup();
  }
}

void loop() {
  if (PROJECT_NAME == "Lablights") {
    litArray();
    callLoop();
    serverLoop(); // web server on http://esp32.local


    if (millis() - prevBlink >= blinkDelay) {
      digitalWrite(PIN_BL, !digitalRead(PIN_BL));
      prevBlink = millis();
    }

    if (millis() - pollStart >= pollInterval) {
      pollStart = millis();
      snmpLoopFinished = false;
      if (SNMPDEBUG) {printVariableHeader();}
      snmpLoop(arrays);

      if (isConnectedToSwitch()) {
        for (int i = 0; i < NUM_CHANNELS; ++i) {
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
      snmpLoopFinished = true;
    }
    if (snmpLoopFinished) {
      for (int i = 0; i < NUM_CHANNELS; ++i) {
        if (pulsesToSendForward[i] > 0) {
          sendPulse(forwardColor[i], i, &pulsesSentForward[i], pulsesToSendForward[i], &previousPulseMillis[i*2], outPulseInterval[i], 0);
        }
        if (pulsesToSendReverse[i] > 0) {
          sendPulse(reverseColor[i], i, &pulsesSentReverse[i], pulsesToSendReverse[i], &previousPulseMillis[i*2+1], inPulseInterval[i], 1);
        }
      }
    }
  }
}

void sendPulse(CRGB color, int strip, unsigned int *pulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction) {
  unsigned long currentMillis = millis();
  if (currentMillis - *prevMillisOfPulse >= interval) {
    *prevMillisOfPulse = currentMillis;
    if (*pulsesSentVar < pulsesMaxSend) {
      pulseEvent(color, strip, direction);
    }
  }
}

void vectorInsert() {
  arrays.insert(arrays.end(), {ARRAY_1});
  arrays.insert(arrays.end(), {ARRAY_2});
  arrays.insert(arrays.end(), {ARRAY_3});
  arrays.insert(arrays.end(), {ARRAY_4});
}