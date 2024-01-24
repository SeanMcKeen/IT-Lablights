/*
This example uses SNMP to poll the number of packets flowing in and out of a network Switch. (with a capitol S because "switch" is a bad word)
We are using an ESP32 here, but it could be updated for an ESP8266 easily enough, but the number of ports we can poll will be smaller - I think.

You will need to add your wifi SSID and password, as well as the IP address of your router/Switch, and the number of ports you want to poll.

*/

//#include "Arduino.h"
#include "WiFi.h" // ESP32 Core Wifi Library
#include <WiFiUdp.h> // enables UDP protocol
#include <Arduino_SNMP_Manager.h> 
#include <snmpgrab.h>
#include <globals.h>
#include <mathhandler.h>

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

const int Array1Count = NUMOFPORTS1;
// How often should you poll the device? 
// int pollInterval = POLL_DELAY; // polling interval (delay) in milliseconds - 1000 = 1 second

// Now we set up our OID (Object Identifiers) variables for the items we want to query on our Switch
// If you don't know what SNMP, MIBs and OIDs are, you can learn more about them here https://www.paessler.com/info/snmp_mibs_and_oids_an_overview
// We'll use arrays for some to store the multiple values of our lastInOctets, responseInOctets and oids
const char *oidSysName = ".1.3.6.1.2.1.1.5.0";       // This is the OID string we query to get the system name (SysName) of the Switch. 
const char *oidUptime = ".1.3.6.1.2.1.1.3.0";        // This OID gets us the uptime of the Switch (hundredths of seconds)

unsigned int responseInOctets1[Array1Count] = {0};  // This will create a resizable array as big as the numberOfPorts we want to poll established above.
unsigned int responseOutOctets1[Array1Count] = {0}; // We need arrays for in and out. 
unsigned int lastOutOctets1[Array1Count] = {0};     // The 'response' arrays will store the data we get from our query, and the 'last' arrays store the value
unsigned int lastInOctets1[Array1Count] = {0};      // from the last time it was polled so we can compare against.
unsigned int in1[Array1Count] = {0};                // These two items store the difference. 
unsigned int out1[Array1Count] = {0};
const char* oidInOctets1[Array1Count];  // We will need to populate this array with the OID strings for the ifInOctets (and out) for each of our ports
const char* oidOutOctets1[Array1Count]; // and we have to do that in setup


char sysName[50]; // empty string thats big enough for 50 characters I guess
char *sysNameResponse = sysName; // will be replaced once we get a response
unsigned int uptime = 0; 
unsigned int lastUptime = 0; 
// unsigned long pollStart = 0;
// unsigned long intervalBetweenPolls = 0;
int in1Total = 0;
int out1Total = 0;
int arr1Totals[2] = {};

int in2Total = 0;
int out2Total = 0;
int arr2Totals[2] = {};

// SNMP Objects
WiFiUDP udp;                                           // UDP object used to send and receive packets
SNMPManager snmp = SNMPManager(community);             // Starts an SNMPManager to listen to replies to get-requests
SNMPGet snmpRequest = SNMPGet(community, snmpVersion); // Starts an SNMPGet instance to send requests
// Blank callback pointer for each OID
ValueCallback *callbackInOctets[numberOfPorts] = {0};  // These are the callback handlers for the various items we are gathering
ValueCallback *callbackOutOctets[numberOfPorts] = {0};
ValueCallback *callbackSysName;
ValueCallback *callbackUptime;

// Declare some empty functions? not sure why we do this... 

// void createArrays();
void snmpLoop(int Array[], int arrayCount, int arrayIndex);
void getInSNMP(int Array[], int arrayCount);
void getOutSNMP(int Array[], int arrayCount);
void handleAllOutputs(int Array[], int arrayCount, int arrayIndex);
void setTotals(int arrayIndex);

void printVariableHeader();
void printVariableFooter();

void SNMPsetup(int Array[], int arrayCount)
{
  snmp.setUDP(&udp); // give snmp a pointer to the UDP object
  snmp.begin();      // start the SNMP Manager

  // Get callbacks from creating a handler for each of the OID
  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i];
    std::string oidInStr = ".1.3.6.1.2.1.2.2.1.10." + std::to_string(o); // create the list of inOids 
    std::string oidOutStr = ".1.3.6.1.2.1.2.2.1.16." + std::to_string(o); // create the list of outOids
    oidInOctets1[o] = oidInStr.c_str();
    oidOutOctets1[o] = oidOutStr.c_str();
    callbackInOctets[o]= snmp.addCounter32Handler(Switch, oidInOctets1[o], &responseInOctets1[o]); // create callbacks array for the OID
    callbackOutOctets[o]= snmp.addCounter32Handler(Switch, oidOutOctets1[o], &responseOutOctets1[o]); // create callbacks array for the OID
  }
  callbackSysName = snmp.addStringHandler(Switch, oidSysName, &sysNameResponse);
  callbackUptime = snmp.addTimestampHandler(Switch, oidUptime, &uptime);
}

void snmpLoop(int Array[], int arrayCount, int arrayIndex){ // the port array, the amount of values in the array, and which array it is: 1st array would be 1, etc.
  snmp.loop();
  getInSNMP(Array, arrayCount);
  getOutSNMP(Array, arrayCount);
  handleAllOutputs(Array, arrayCount, arrayIndex);
  setTotals(arrayIndex);
}

void handleAllOutputs(int Array[], int arrayCount, int arrayIndex){
  in1Total = 0;
  out1Total = 0;

  int* variableToUseIN;
  int* variableToUseOUT;
  

  if (arrayIndex == 1) {
    variableToUseIN = &in1Total;
    variableToUseOUT = &out1Total;
  }else if(arrayIndex == 2){
    variableToUseIN = &in2Total;
    variableToUseOUT = &out2Total;
  }

  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i]; // iterate positions through our array of ports, EX: position 0 is the first number in our ports to check array
    in1[o] = responseInOctets1[o]-lastInOctets1[o];
    variableToUseIN += in1[o];
    lastInOctets1[o] = responseInOctets1[o];
  }
  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i];
    out1[o] = responseOutOctets1[o]-lastOutOctets1[o];
    variableToUseOUT += out1[o];
    lastOutOctets1[o] = responseOutOctets1[o];
  }
}

void setTotals(int arrayIndex){
  int* varToUseIN;
  int* varToUseOUT;

  if (arrayIndex == 1){
    varToUseIN = &in1Total;
    varToUseOUT = &out1Total;
  }else if (arrayIndex == 2){
    varToUseIN = &in2Total;
    varToUseOUT = &out2Total;
  }
  for (int i; i < 2; i++){
    if (i == 0){
      arr1Totals[i] = *varToUseIN;
    }else if (i == 1){
      arr1Totals[i] = *varToUseOUT;
    }
  }
}

void getInSNMP(int Array[], int arrayCount)
{
  for (int i = 0; i < arrayCount; ++i) {
  int o = Array[i];
  snmpRequest.addOIDPointer(callbackInOctets[o]);
  }
  snmpRequest.addOIDPointer(callbackSysName);
  snmpRequest.addOIDPointer(callbackUptime);
  snmpRequest.setIP(WiFi.localIP()); // IP of the listening MCU
  snmpRequest.setUDP(&udp);
  snmpRequest.setRequestID(rand() % 5555);
  snmpRequest.sendTo(Switch);
  snmpRequest.clearOIDList();
}

void getOutSNMP(int Array[], int arrayCount)
{
  for (int i = 0; i < arrayCount; ++i) {
  int o = Array[i];
  snmpRequest.addOIDPointer(callbackOutOctets[o]);
  }
  snmpRequest.addOIDPointer(callbackSysName);
  snmpRequest.addOIDPointer(callbackUptime);
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
}




