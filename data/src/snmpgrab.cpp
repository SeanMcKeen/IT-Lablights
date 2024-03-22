/*
This example uses SNMP to poll the number of packets flowing in and out of a network Switch. (with a capitol S because "switch" is a bad word)
We are using an ESP32 here, but it could be updated for an ESP8266 easily enough, but the number of ports we can poll will be smaller - I think.

You will need to add your wifi SSID and password, as well as the IP address of your router/Switch, and the number of ports you want to poll.

*/

//#include "Arduino.h"
#include "WiFi.h" // ESP32 Core Wifi Library
#include <WiFiUdp.h> // enables UDP protocol
#include <Arduino_SNMP_Manager.h>
#include <globals.h>
#include <mathhandler.h>
#include <vector>

#if LABLIGHTS
#include <lablights.h>
#endif

// Your WiFi info 
const char *ssid = "ITLAB";
const char *password = "letmeinnow";

// Insert your SNMP Device Info 
IPAddress Switch(SWITCH_IP);  // must capitalize the letter S for our variable
const char *community = "public"; // if different from the default of "public"
const int snmpVersion = 1; // SNMP Version 1 = 0, SNMP Version 2 = 1

// How many ports?
const int numberOfPorts = NUM_PORTS; // Set this to the desired number of ports, e.g., 48 in this case
// CAUTION: We seem to hit a maximum of how many ports can be polled at one time (30). if you experience device reboots, you are probably asking for too many OIDs

// Now we set up our OID (Object Identifiers) variables for the items we want to query on our Switch
// If you don't know what SNMP, MIBs and OIDs are, you can learn more about them here https://www.paessler.com/info/snmp_mibs_and_oids_an_overview
// We'll use arrays for some to store the multiple values of our lastInOctets, responseInOctets and oids
const char *oidSysName = ".1.3.6.1.2.1.1.5.0";       // This is the OID string we query to get the system name (SysName) of the Switch. 
const char *oidUptime = ".1.3.6.1.2.1.1.3.0";        // This OID gets us the uptime of the Switch (hundredths of seconds)

unsigned int responseInOctets[numberOfPorts+1] = {0};  // This will create a resizable array as big as the numberOfPorts we want to poll established above.
unsigned int responseOutOctets[numberOfPorts+1] = {0}; // We need arrays for in and out. 

int lastOutOctets[numberOfPorts+1] = {0};     // The 'response' arrays will store the data we get from our query, and the 'last' arrays store the value
int lastInOctets[numberOfPorts+1] = {0};      // from the last time it was polled so we can compare against.

const char* oidInOctets[numberOfPorts+1];  // We will need to populate this array with the OID strings for the ifInOctets (and out) for each of our ports
const char* oidOutOctets[numberOfPorts+1]; // and we have to do that in setup

char sysName[11]; // empty string thats big enough for 50 characters I guess
char *sysNameResponse = sysName; // will be replaced once we get a response
unsigned int uptime = 0; 
unsigned int lastUptime = 0; 

int tempArrTotals[NUM_CHANNELS][2] = {{0}};
int arrTotals[NUM_CHANNELS][2] = {{0}};

// SNMP Objects
WiFiUDP udp;                                           // UDP object used to send and receive packets
SNMPManager snmp = SNMPManager(community);             // Starts an SNMPManager to listen to replies to get-requests
SNMPGet snmpRequest = SNMPGet(community, snmpVersion); // Starts an SNMPGet instance to send requests
// Blank callback pointer for each OID
ValueCallback *callbackInOctets[numberOfPorts+1] = {0};  // These are the callback handlers for the various items we are gathering
ValueCallback *callbackOutOctets[numberOfPorts+1] = {0};
ValueCallback *callbackSysName = {0};
ValueCallback *callbackUptime = {0};

// Declare some empty functions? not sure why we do this... 

// void createArrays();
void snmpLoop(const std::vector<int>& ports);
void getInSNMP(const std::vector<int>& ports);
void getOutSNMP(const std::vector<int>& ports);
void handleAllOutputs(const std::vector<int>& ports);
void setTotals();
int findPortArrayIndex(int port);

void printVariableHeader();
void printVariableFooter();

void SNMPsetup(const std::vector<int>& ports)
{
  snmp.setUDP(&udp); // give snmp a pointer to the UDP object
  snmp.begin();      // start the SNMP Manager

  // Get callbacks from creating a handler for each of the OID
  for (int port : ports) {
    std::string oidInStr = ".1.3.6.1.2.1.2.2.1.10." + std::to_string(port); // create the list of inOids 
    std::string oidOutStr = ".1.3.6.1.2.1.2.2.1.16." + std::to_string(port); // create the list of outOids
    // why is there a 1 in these? oidInOctets1?  Mr D
    oidInOctets[port] = oidInStr.c_str();
    oidOutOctets[port] = oidOutStr.c_str();
    callbackInOctets[port]= snmp.addCounter32Handler(Switch, oidInOctets[port], &responseInOctets[port]); // create callbacks array for the OID
    callbackOutOctets[port]= snmp.addCounter32Handler(Switch, oidOutOctets[port], &responseOutOctets[port]); // create callbacks array for the OID
  }
  callbackSysName = snmp.addStringHandler(Switch, oidSysName, &sysNameResponse);
  callbackUptime = snmp.addTimestampHandler(Switch, oidUptime, &uptime);
}

void callLoop(){
  snmp.loop();
}

void snmpLoop(const std::vector<int>& ports){ // the port array, the amount of values in the array, and which array it is: 1st array would be 1, etc.
  getInSNMP(ports);
  getOutSNMP(ports);
  handleAllOutputs(ports);
  setTotals();
}

void handleAllOutputs(const std::vector<int>& ports){
  // Reset temp Total variables so that the current data isn't mixing with the last poll.
  for (int i = 0; i < NUM_CHANNELS; i++) {
    tempArrTotals[i][0] = 0;
    tempArrTotals[i][1] = 0;
  }

  // Prints and calculates the ports IN data: ---------------------------------------------
  for (int port : ports) {
    int subT; // to store the difference between response and last response

    subT = responseInOctets[port] - lastInOctets[port];
    if (subT < 0) {subT = 0;} else if (subT > 10000000) {subT = 10000000;}
    int index = findPortArrayIndex(port);
    tempArrTotals[index][0] += subT;
    lastInOctets[port] = responseInOctets[port]; // set the last to the response, after this is where response can be redefined and it wont matter.

    if(SNMPDEBUG){
      Serial.println();
      Serial.printf("Port %i IN: ", port);
      Serial.print(subT);
    }
  }

  // Just to get a line between IN and OUT
  if(SNMPDEBUG){Serial.println();}

  // Prints and calculates the ports OUT data: -------------------------------------------------
  for (int port : ports) {
    int subT;

    subT = responseOutOctets[port] - lastOutOctets[port];
    if (subT < 0) {subT = 0;} else if (subT > 10000000) {subT = 10000000;}
    int index = findPortArrayIndex(port);
    tempArrTotals[index][1] += subT;
    lastOutOctets[port] = responseOutOctets[port]; // set the last to the response, after this is where response can be redefined and it wont matter.

    if (SNMPDEBUG) {
      Serial.println();
      Serial.printf("Port %i OUT: ", port);
      Serial.print(subT);
    }
  }

  if(SNMPDEBUG){Serial.println();}
}

void setTotals(){ // This is where we actually set the variables to pull from in our main.
  for (int i = 0; i < NUM_CHANNELS; i++) {
    arrTotals[i][0] = tempArrTotals[i][0];
    arrTotals[i][1] = tempArrTotals[i][1];
  }
}

void getInSNMP(const std::vector<int>& ports) // This is a lot of stuff I don't understand, so just research snmp coding and you might understand.
{
  for (int port : ports) {
    snmpRequest.addOIDPointer(callbackInOctets[port]);
  }
  snmpRequest.addOIDPointer(callbackSysName);
  snmpRequest.addOIDPointer(callbackUptime);
  snmpRequest.setIP(WiFi.localIP()); // IP of the listening MCU
  snmpRequest.setUDP(&udp);
  snmpRequest.setRequestID(rand() % 5555);
  snmpRequest.sendTo(Switch);
  snmpRequest.clearOIDList();
}

void getOutSNMP(const std::vector<int>& ports) // Again I have no clue, this was part of the sample code in the library I used.
{
  for (int port : ports) {
    snmpRequest.addOIDPointer(callbackOutOctets[port]);
  }
  snmpRequest.setIP(WiFi.localIP()); // IP of the listening MCU
  snmpRequest.setUDP(&udp);
  snmpRequest.setRequestID(rand() % 5555);
  snmpRequest.sendTo(Switch);
  snmpRequest.clearOIDList();
}

void printVariableHeader()
{ // just the header really.
  Serial.print("My IP: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Polling Device: %s\n", sysNameResponse);
  Serial.printf("Uptime: %d\n", uptime);
  Serial.println("----------------------");
}

void printVariableFooter()
{
  unsigned long currentTime = millis();
  Serial.print("----- elapsed: ");
  Serial.print(currentTime - lastUptime);
  Serial.println(" -----");
  // Update last samples
  lastUptime = currentTime;
  Serial.println();
  // Some debugging information
  Serial.printf("Strip 1 Averaged, IN: %i  OUT: %i", arrTotals[0][0], arrTotals[0][1]);
  Serial.println();
  if (Strip2){
    Serial.printf("Strip 2 Averaged, IN: %i  OUT: %i", arrTotals[1][0], arrTotals[1][1]);
    Serial.println();
  }
  if (Strip3){
    Serial.printf("Strip 3 Averaged, IN: %i  OUT: %i", arrTotals[2][0], arrTotals[2][1]);
    Serial.println();
  }
  if (Strip4){
    Serial.printf("Strip 4 Averaged, IN: %i  OUT: %i", arrTotals[3][0], arrTotals[3][1]);
    Serial.println();
  }
}

int findPortArrayIndex(int port) {
    const int arrays[][2] = { {ARRAY_1}, {ARRAY_2}, {ARRAY_3}, {ARRAY_4} };
    const int numArrays = sizeof(arrays) / sizeof(arrays[0]);

    for (int i = 0; i < numArrays; ++i) {
        const int* currentArray = arrays[i];
        const int arraySize = 2; // Since each ARRAY_X has two elements
        for (int j = 0; j < arraySize; ++j) {
            if (port == currentArray[j]) {
                return i;
            }
        }
    }

    // Port is not found in any array
    return -1; // or any other indication of failure
}
