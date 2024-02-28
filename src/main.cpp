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
unsigned long prevPollMillis = 0;
unsigned long intervalBetweenPolls = 0;

int channelAmount = NUM_CHANNELS;

// Thank you to the person in reddit for suggesting I use arrays!!
// PulseInterval Variables (Must have a forward and reverse for each strip)
unsigned long inPulseInterval[NUM_CHANNELS+1] = {};

unsigned long outPulseInterval[NUM_CHANNELS+1] = {};

//We need a previous pulse for each strips' forward and backward, AKA 2*NUM_CHANNELS
unsigned long previousPulseMillis[NUM_CHANNELS*2] = {};

// PulsesToSend Variables (Must have a forward and reverse for each strip)
int pulsesToSendForward[NUM_CHANNELS+1] = {};
int pulsesToSendReverse[NUM_CHANNELS+1] = {};

// PulsesSent Variables (Must have a forward and reverse for each strip)
int pulsesSentForward[NUM_CHANNELS+1] = {};
int pulsesSentReverse[NUM_CHANNELS+1] = {};

CRGB forwardColor[NUM_CHANNELS+1] = {CRGB::Blue};
CRGB reverseColor[NUM_CHANNELS+1] = {CRGB::Blue};

unsigned long InAvg[NUM_PORTS+1] = {};
unsigned long OutAvg[NUM_PORTS+1] = {};

#if LABLIGHTS // Here's where we include Lablights ONLY header files, saves space and time in case we want to build a different project
#include "enablewifi.h"
#include "lablights.h"
#include "mathhandler.h"
#endif

std::vector<int> MainArray;

// define functions
void sendPulses();
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
    unsigned long currentMillis = millis();
    if (currentMillis - prevPollMillis >= pollInterval){
      
      if (SNMPDEBUG){
        size_t freeHeap = esp_get_free_heap_size();
        Serial.printf("Free heap size: %d bytes\n", freeHeap);
      }

      if (SNMPDEBUG){printVariableHeader();} // Another func from snmp.cpp, prints debugging data in serial output
      // This block establishes all variables for strip 1 pulses
      // snmpLoop(MainArray);

      unsigned long long int thisINTotal = 0;
      unsigned long long int thisOUTTotal = 0;

      for (int y : MainArray){
        InAvg[y] = 500000;
        OutAvg[y] = 500000;

        // InAvg[y] = arrINTotals[y];
        // OutAvg[y] = arrOUTTotals[y];

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
    interval = outPulseInterval[strip];
    color = forwardColor[strip];
  }else{
    interval = inPulseInterval[strip];
    color = reverseColor[strip];
  }

  if (currentMillis - previousPulseMillis[prev] >= interval){
    previousPulseMillis[prev] = currentMillis;
  if (direction == 0){
    if (pulsesSentForward[strip] < pulsesToSendForward[strip]){
      pulseEvent(color, strip, direction);
    }
  }else{
    if (pulsesSentReverse[strip] < pulsesToSendReverse[strip]){
      pulseEvent(color, strip, direction);
    }
  }
  }
}

void sendPulses() {
  for (int i = 1; i <= NUM_CHANNELS; i++) {
      sendPulse(i, 0, (i-1)*2);
      sendPulse(i, 1, (i-1)*2+1);
  }
}

int getIndex(int y) {
  if (inArray(Array1, y)) return 1;
  else if (inArray(Array2, y)) return 2;
  else if (inArray(Array3, y)) return 3;
  else if (inArray(Array4, y)) return 4;
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
  if (SNMPDEBUG){
    Serial.printf("Strip %i Averaged, IN: %llu  OUT: %llu\n", stripNum, TotalsIN, TotalsOUT);
  }
}

void printVariableFooter() {
  unsigned long currentTime = millis();
  if (SNMPDEBUG){
    Serial.print("----- elapsed: ");
    Serial.print(currentTime - lastUptime);
    Serial.println(" -----");
  }
  updateSNMPUptime();
  if (SNMPDEBUG){Serial.println();}

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