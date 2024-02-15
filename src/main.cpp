#include "globals.h"
#include "secrets.h"
#include "vector"
#include "snmpgrab.h"
#include "algorithm"
// Place header files above, headers are like pointers to other files in the project. 
// Headers must include function definitions for functions you want to use in other files. 
// Header files SHOULD be named the same as the cpp file they correspond to.

// Main Code Section -- Executed no matter what

// Here we define our globals.h variables into other variables.
std::vector<int> Array1 = ARRAY_1;
std::vector<int> Array2 = ARRAY_2;
std::vector<int> Array3 = ARRAY_3;
std::vector<int> Array4 = ARRAY_4;

// Defining local variables (These are set to 0 as they are redefined later in the code)
int pollInterval = POLL_DELAY;
int pollTiming = pollInterval/1000;
unsigned long pollStart = 0;
unsigned long intervalBetweenPolls = 0;

int channelAmount = NUM_CHANNELS;

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

CRGB forwardColor[NUM_CHANNELS] = {CRGB::Blue};
CRGB reverseColor[NUM_CHANNELS] = {CRGB::Blue};

unsigned long InAvg[NUM_PORTS] = {};
unsigned long OutAvg[NUM_PORTS] = {};

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include "enablewifi.h"
#include "lablights.h"
#include "mathhandler.h"
#endif

std::vector<int> MainArray;

// define functions
void sendPulse(CRGB color, int strip, int *PulsesSentVar, int pulsesMaxSend, unsigned long *prevMillisOfPulse, unsigned long interval, int direction);
void printVariableFooter();
void makeArrays();
bool inArray(std::vector<int> arr, int num);
int getIndex(int y);

void setup() { // Main Setup
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    WifiBegin();
    makeArrays();
    SNMPsetup(MainArray);
    initFastLED();
  }
}

// Main loop
void loop() {
  if (PROJECT_NAME == "Lablights") {
    callLoop();
    litArray();
    intervalBetweenPolls = millis() - pollStart;
    if (intervalBetweenPolls >= pollInterval) {
      pollStart += pollInterval; // this prevents drift in the delays

      size_t freeHeap = esp_get_free_heap_size();
      printf("Free heap size: %d bytes\n", freeHeap);

      printVariableHeader(); // Another func from snmp.cpp, prints debugging data in serial output
      // This block establishes all variables for strip 1 pulses
      snmpLoop(MainArray);

      unsigned long long int thisINTotal = 0;
      unsigned long long int thisOUTTotal = 0;

      for (int y : MainArray){
        InAvg[y] = arrINTotals[y];
        OutAvg[y] = arrOUTTotals[y]; // might be wrong

        int index = getIndex(y);

        if (index != -1){
          pulsesToSendForward[index] = calcSNMPPulses(OutAvg[index]);
          forwardColor[index] = calcPulseColor(OutAvg[index]);
          outPulseInterval[index] = (pollTiming/pulsesToSendForward[index] * 1000);

          pulsesToSendReverse[index] = calcSNMPPulses(InAvg[index]);
          reverseColor[index] = calcPulseColor2(InAvg[index]);
          inPulseInterval[index] = (pollTiming/pulsesToSendReverse[index] * 1000);

          pulsesSentForward[index] = 0;
          pulsesSentReverse[index] = 0;
        }
      }
      printVariableFooter();
    }
    // handle sending the pulses every x seconds
    // EACH pulse needs its own set of variables if you want it to be independent
    // Essentially we want each strip handling an array, since we have 4 strips, it would be appropriate to have 4 arrays.
    // sendPulse(forwardColor[0], 1, &pulsesSentForward[0], pulsesToSendForward[0], &previousPulseMillis[0], outPulseInterval[0], 0); // call a pulse forward/OUT
    // sendPulse(reverseColor[0], 1, &pulsesSentReverse[0], pulsesToSendReverse[0], &previousPulseMillis[1], inPulseInterval[0], 1); // call a pulse reverse/IN

    // sendPulse(forwardColor[1], 2, &pulsesSentForward[1], pulsesToSendForward[1], &previousPulseMillis[2], outPulseInterval[1], 0);
    // sendPulse(reverseColor[1], 2, &pulsesSentReverse[1], pulsesToSendReverse[1], &previousPulseMillis[3], inPulseInterval[1], 1);

    // sendPulse(forwardColor[2], 3, &pulsesSentForward[2], pulsesToSendForward[2], &previousPulseMillis[4], outPulseInterval[2], 0);
    // sendPulse(reverseColor[2], 3, &pulsesSentReverse[2], pulsesToSendReverse[2], &previousPulseMillis[5], inPulseInterval[2], 1);

    // sendPulse(forwardColor[3], 4, &pulsesSentForward[3], pulsesToSendForward[3], &previousPulseMillis[6], outPulseInterval[3], 0);
    // sendPulse(reverseColor[3], 4, &pulsesSentReverse[3], pulsesToSendReverse[3], &previousPulseMillis[7], inPulseInterval[3], 1);
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

int getIndex(int y) {
  if (inArray(Array1, y)) return 0;
  else if (inArray(Array2, y)) return 1;
  else if (inArray(Array3, y)) return 2;
  else if (inArray(Array4, y)) return 3;
  return -1;
}

void makeArrays() {
  std::vector<std::vector<int>> arrays = {Array1, Array2, Array3, Array4};
  for (const auto& arr : arrays) {
    for (int i : arr) {
      MainArray.push_back(i);
    }
  }
}

void printStripAverages(int stripNum, std::vector<int>& arr) {
  long long unsigned TotalsIN = 0;
  long long unsigned TotalsOUT = 0;
  for (int i : arr) {
    TotalsIN += arrINTotals[i];
    TotalsOUT += arrOUTTotals[i];
  }
  Serial.printf("Strip %i Averaged, IN: %llu  OUT: %llu\n", stripNum, TotalsIN, TotalsOUT);
}

void printVariableFooter() {
  unsigned long currentTime = millis();
  Serial.print("----- elapsed: ");
  Serial.print(currentTime - lastUptime);
  Serial.println(" -----");
  updateSNMPUptime();
  Serial.println();

  std::vector<std::vector<int>> stripArrays = {Array1, Array2, Array3, Array4};
  std::vector<bool> strips = {true, Strip2, Strip3, Strip4};

  for (int i = 0; i < strips.size(); ++i) {
    if (strips[i]) {
      printStripAverages(i + 1, stripArrays[i]);
    }
  }
}

bool inArray(std::vector<int> arr, int num) {
  for (int i = 0; i < arr.size(); ++i) {
    if (arr[i] == num) {
        return true;
    }
  }
  return false; // Return false if num is not found in the array
}