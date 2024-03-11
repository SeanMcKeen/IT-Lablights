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
unsigned long inPulseInterval[NUM_CHANNELS] = {0};

unsigned long outPulseInterval[NUM_CHANNELS] = {0};

//We need a previous pulse for each strips' forward and backward, AKA 2*NUM_CHANNELS
unsigned long previousPulseMillis[NUM_CHANNELS*2] = {0};

// PulsesToSend Variables (Must have a forward and reverse for each strip)
int pulsesToSendForward[NUM_CHANNELS] = {0};
int pulsesToSendReverse[NUM_CHANNELS] = {0};

// PulsesSent Variables (Must have a forward and reverse for each strip)
int pulsesSentForward[NUM_CHANNELS] = {0};
int pulsesSentReverse[NUM_CHANNELS] = {0};

CRGB forwardColor[NUM_CHANNELS] = {CRGB::Blue};
CRGB reverseColor[NUM_CHANNELS] = {CRGB::Blue};

unsigned long Ins[NUM_PORTS+1] = {0};
unsigned long Outs[NUM_PORTS+1] = {0};

unsigned long ArrIns[NUM_CHANNELS] = {0};
unsigned long ArrOuts[NUM_CHANNELS] = {0};

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
void clearData();

void setup() { // Main Setup
  if (PROJECT_NAME == "Lablights") { // Again, setup exclusive to Lablights
    Serial.begin(115200); // Begin serial monitor, so we can write outputs
    WifiBegin();
    makeArrays();
    // SNMPsetup(MainArray);
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
      if (SNMPDEBUG){
        size_t freeHeap = esp_get_free_heap_size();
        Serial.printf("Free heap size: %d bytes\n", freeHeap);
      }

      // if (SNMPDEBUG){printVariableHeader();} // Another func from snmp.cpp, prints debugging data in serial output
      // This block establishes all variables for strip 1 pulses
      // snmpLoop(MainArray);
      int index;

      clearData();

      for (int y : MainArray){
        // Serial.println(y); // I'm getting 0's after the initial run??
        Ins[y] = 500000;
        Outs[y] = 500000;

        // InAvg[y] = arrINTotals[y];
        // OutAvg[y] = arrOUTTotals[y];
        index = getIndex(y);
        if (index >= 0){
          ArrIns[index] += Ins[index];
          ArrOuts[index] += Outs[index];
        }
      }
      for (int i = 0; i < NUM_CHANNELS; i++){ // All variables will be index 0-3 unless holding/controlling ports
        Serial.printf("111, %i\n", i);
        pulsesToSendForward[3] = calcSNMPPulses(ArrOuts[i]); // why does this crash on index 3????
        // forwardColor[i] = calcPulseColor(ArrOuts[i]);

        // pulsesToSendReverse[i] = calcSNMPPulses(ArrIns[i]);
        // reverseColor[i] = calcPulseColor2(ArrIns[i]);
        // outPulseInterval[i] = (pollTiming/pulsesToSendForward[i] * 1000);
        // inPulseInterval[i] = (pollTiming/pulsesToSendReverse[i] * 1000);

        // pulsesSentForward[i] = 0;
        // pulsesSentReverse[i] = 0;

      }
      // printVariableFooter();
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
  for (int i = 1; i <= NUM_CHANNELS; i++) {
      sendPulse(i, 0, (i-1)*2);
      sendPulse(i, 1, (i-1)*2+1);
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

void clearData(){
  for (int i = 1; i <= NUM_CHANNELS; i++){
    pulsesToSendForward[i] = 0;
    forwardColor[i] = 0;
    pulsesToSendReverse[i] = 0;
    reverseColor[i] = 0;
    outPulseInterval[i] = 0;
    inPulseInterval[i] = 0;
    Ins[i] = 0;
    Outs[i] = 0;
  }
}