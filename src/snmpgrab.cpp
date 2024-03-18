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


char sysName[50]; // empty string thats big enough for 50 characters I guess
char *sysNameResponse = sysName; // will be replaced once we get a response
unsigned int uptime = 0; 
unsigned int lastUptime = 0; 
// unsigned long pollStart = 0;
// unsigned long intervalBetweenPolls = 0;
int in1Total = 0;
int out1Total = 0;

// We'll leave these established as it wont really affect speed, and would cause issues to try to IF define them.
// Need a block for each strip
int in2Total = 0;
int out2Total = 0;

int in3Total = 0;
int out3Total = 0;

int in4Total = 0;
int out4Total = 0;

int arrTotals[NUM_CHANNELS][2];

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
    // why is there a 1 in these? oidInOctets1?  Mr D
    oidInOctets[o] = oidInStr.c_str();
    oidOutOctets[o] = oidOutStr.c_str();
    callbackInOctets[o]= snmp.addCounter32Handler(Switch, oidInOctets[o], &responseInOctets[o]); // create callbacks array for the OID
    callbackOutOctets[o]= snmp.addCounter32Handler(Switch, oidOutOctets[o], &responseOutOctets[o]); // create callbacks array for the OID
  }
  callbackSysName = snmp.addStringHandler(Switch, oidSysName, &sysNameResponse);
  callbackUptime = snmp.addTimestampHandler(Switch, oidUptime, &uptime);
}

void callLoop(){
  snmp.loop();
}

void snmpLoop(int Array[], int arrayCount, int arrayIndex){ // the port array, the amount of values in the array, and which array it is: 1st array would be 1, etc.
  getInSNMP(Array, arrayCount);
  getOutSNMP(Array, arrayCount);
  handleAllOutputs(Array, arrayCount, arrayIndex);
  setTotals(arrayIndex);
}

void handleAllOutputs(int Array[], int arrayCount, int arrayIndex){
  in1Total = 0; // Reset all Total variables so that the current data isn't mixing with the last poll.
  in2Total = 0;
  in3Total = 0;
  in4Total = 0;

  out1Total = 0;
  out2Total = 0;
  out3Total = 0;
  out4Total = 0;

  for (int i = 0; i < arrayCount; i++) {
    int o = Array[i]; // i counts from 0 to the maximum in our array - 1, so Array[i] gives us each number in the array we pass to the function.

    int subT; // to store the difference between response and last response

    // Debugging: Print the values to help identify issues
    // Some data displays as negative when first uploading, but it balances out after you give it a few polls

    if (arrayIndex == 1){ // If on the first strip
      subT = responseInOctets[o] - lastInOctets1[o]; // We only need lastInOctets variables to be exclusive, response is changed each time so we don't need a response1, 2, etc.
      in1Total += subT; // add the difference to the total, this is what we actually pull from in our main.cpp
      lastInOctets[o] = responseInOctets[o]; // set the last to the response, after this is where response can be redefined and it wont matter.
    }else if (arrayIndex == 2){ // repeat for strip 2
      subT = responseInOctets[o] - lastInOctets2[o];
      in2Total += subT;
      lastInOctets[o] = responseInOctets[o];
    }else if (arrayIndex == 3){ // repeat for strip 2
      subT = responseInOctets[o] - lastInOctets3[o];
      in3Total += subT;
      lastInOctets[o] = responseInOctets[o];
    }else if (arrayIndex == 4){ // repeat for strip 2
      subT = responseInOctets[o] - lastInOctets4[o];
      in4Total += subT;
      lastInOctets[o] = responseInOctets[o];
    }

    if(SNMPDEBUG){
      Serial.println();
      Serial.printf("Port %i IN: ", o);
      Serial.print(subT);
    }
  }
  // Just to get a line between IN and OUT
  if(SNMPDEBUG){Serial.println();}

  // Prints and calculates the ports OUT data:
  for (int i = 0; i < arrayCount; ++i) {
    int o = Array[i];
    int subT;
    if (arrayIndex == 1){
      subT = responseOutOctets[o]-lastOutOctets1[o];
      out1Total += subT;
      lastOutOctets[o] = responseOutOctets[o];
    }else if (arrayIndex == 2){
      subT = responseOutOctets[o]-lastOutOctets2[o];
      out2Total += subT;
      lastOutOctets[o] = responseOutOctets[o];
    }else if (arrayIndex == 3){
      subT = responseOutOctets[o]-lastOutOctets3[o];
      out3Total += subT;
      lastOutOctets[o] = responseOutOctets[o];
    }else if (arrayIndex == 4){
      subT = responseOutOctets[o]-lastOutOctets4[o];
      out4Total += subT;
      lastOutOctets[o] = responseOutOctets[o];
    }
    if (SNMPDEBUG) {
      Serial.println();
      Serial.printf("Port %i OUT: ", o);
      Serial.print(subT);
    }
  }
  if(SNMPDEBUG){Serial.println();}
}

void setTotals(int arrayIndex){ // This is where we actually set the variables to pull from in our main.
  if (arrayIndex == 1){
    arrTotals[0][0] = in1Total; // We use arr1Totals for the entire strip 1, IN will be at index 0, and OUT will be at index 1.
    arrTotals[0][1] = out1Total;
  }else if (arrayIndex == 2){ // Repeat for strip 2
    arrTotals[1][0] = in2Total;
    arrTotals[1][1] = out2Total;
  }else if (arrayIndex == 3){ // Repeat for strip 3
    arrTotals[2][0] = in3Total;
    arrTotals[2][1] = out3Total;
  }else if (arrayIndex == 4){ // Repeat for strip 4
    arrTotals[3][0] = in4Total;
    arrTotals[3][1] = out4Total;
  }
}

void getInSNMP(int Array[], int arrayCount) // This is a lot of stuff I don't understand, so just research snmp coding and you might understand.
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

void getOutSNMP(int Array[], int arrayCount) // Again I have no clue, this was part of the sample code in the library I used.
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