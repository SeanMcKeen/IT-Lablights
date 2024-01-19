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
unsigned int in1Total = 0;
unsigned int in1Avg = 0;
unsigned int out1Total = 0;
unsigned int out1Avg = 0;
unsigned int getCounter = 0;

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
void getInSNMP(int Array[], int arrayCount);
void getOutSNMP(int Array[], int arrayCount);
void printInOutputs(int Array[], int arrayCount);
void printOutOutputs(int Array[], int arrayCount);
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

int snmpInLoop(int Array[], int arrayCount){
  snmp.loop();
  getInSNMP(Array, arrayCount);
  printInOutputs(Array, arrayCount);
  return in1Avg;
}
int snmpOutLoop(int Array[], int arrayCount){
  snmp.loop();
  getOutSNMP(Array, arrayCount);
  printOutOutputs(Array, arrayCount);
  return out1Avg;
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

void printInOutputs(int Array[], int arrayCount) {
  // We will be receiving a number of snmp responses from our Switch reporting how many octets of data were received and sent for each of the polled ports. 
  // We will query anywhere from one to 48 ports, and assign each response to a variable such as "responseInOctets[0]" for  port 1, and so on.
  // for each of our responses, as responseInOctets[#] subtract lastInOctets[#] from it and assign it to a variable in[#]
  // then print the variable in#, and do the same to out while we are at it.
  in1Avg = 0;
  in1Total = 0;
  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i]; // iterate positions through our array of ports, EX: position 0 is the first number in our ports to check array
    in1[o] = responseInOctets1[o]-lastInOctets1[o];
    in1Total += in1[o];
    Serial.print("Port "); 
    Serial.print(o); 
    Serial.print(" In: ");
    Serial.print(in1[o]);
    Serial.println();
    lastInOctets1[o] = responseInOctets1[o];
  }
  Serial.println();
  in1Avg = in1Total/arrayCount;
}

void printOutOutputs(int Array[], int arrayCount) {
  // We will be receiving a number of snmp responses from our Switch reporting how many octets of data were received and sent for each of the polled ports. 
  // We will query anywhere from one to 48 ports, and assign each response to a variable such as "responseInOctets[0]" for  port 1, and so on.
  // for each of our responses, as responseInOctets[#] subtract lastInOctets[#] from it and assign it to a variable in[#]
  // then print the variable in#, and do the same to out while we are at it.
  out1Avg = 0;
  out1Total = 0;
  Serial.println();
  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i];
    out1[o] = responseOutOctets1[o]-lastOutOctets1[o];
    out1Total += out1[o];
    Serial.print("Port "); 
    Serial.print(o); 
    Serial.print(" Out: ");
    Serial.print(out1[o]);
    lastOutOctets1[o] = responseOutOctets1[o];
    Serial.println();
  }
  Serial.println();
  out1Avg = out1Total/arrayCount;
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
  Serial.printf("In Averaged: %d\n", in1Avg);
  Serial.printf("Out Averaged: %d\n", out1Avg);
  Serial.print("----- elapsed: ");
  Serial.print(uptime - lastUptime);
  Serial.println(" -----");
  // Update last samples
  lastUptime = uptime;
}




