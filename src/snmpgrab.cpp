/*
This example uses SNMP to poll the number of packets flowing in and out of a network Switch. (with a capitol S because "switch" is a bad word)
We are using an ESP32 here, but it could be updated for an ESP8266 easily enough, but the number of ports we can poll will be smaller - I think.

You will need to add your wifi SSID and password, as well as the IP address of your router/Switch, and the number of ports you want to poll.

*/

//#include "Arduino.h"
#include "WiFi.h" // ESP32 Core Wifi Library
#include "WiFiUdp.h" // enables UDP protocol
#include "Arduino_SNMP_Manager.h" 
#include "snmpgrab.h"
#include "globals.h"

#if LABLIGHTS
#include "lablights.h"
#endif

// Your WiFi info 
const char *ssid = "ITLAB";
const char *password = "letmeinnow";

// Insert your SNMP Device Info 
IPAddress Switch(SWITCH_IP);  // must capitalize the letter S for our variable
const char *community = SNMP_COMM; // if different from the default of "public"
const int snmpVersion = 1; // SNMP Version 1 = 0, SNMP Version 2 = 1

// How many ports?
const int numberOfPorts = NUM_PORTS; // Set this to the desired number of ports, e.g., 48 in this case
// CAUTION: We seem to hit a maximum of how many ports can be polled at one time (30). if you experience device reboots, you are probably asking for too many OIDs

// Now we set up our OID (Object Identifiers) variables for the items we want to query on our Switch
// If you don't know what SNMP, MIBs and OIDs are, you can learn more about them here https://www.paessler.com/info/snmp_mibs_and_oids_an_overview
// We'll use arrays for some to store the multiple values of our lastInOctets, responseInOctets and oids
const char *oidSysName = ".1.3.6.1.2.1.1.5.0";       // This is the OID string we query to get the system name (SysName) of the Switch. 
const char *oidUptime = ".1.3.6.1.2.1.1.3.0";        // This OID gets us the uptime of the Switch (hundredths of seconds)

unsigned int responseInOctets[numberOfPorts] = {0};  // This will create a resizable array as big as the numberOfPorts we want to poll established above.
unsigned int responseOutOctets[numberOfPorts] = {0}; // We need arrays for in and out. 

int lastOutOctets[numberOfPorts] = {0};     // The 'response' arrays will store the data we get from our query, and the 'last' arrays store the value
int lastInOctets[numberOfPorts] = {0};      // from the last time it was polled so we can compare against.
const char* oidInOctets[numberOfPorts];  // We will need to populate this array with the OID strings for the ifInOctets (and out) for each of our ports
const char* oidOutOctets[numberOfPorts]; // and we have to do that in setup


char sysName[50]; // empty string thats big enough for 50 characters I guess
char *sysNameResponse = sysName; // will be replaced once we get a response
uint32_t uptime = 0; 
uint32_t lastUptime = 0; 

unsigned long long int arrINTotals[NUM_PORTS+1] = {};
unsigned long long int arrOUTTotals[NUM_PORTS+1] = {};

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
void snmpLoop(const std::vector<int>& Array);
void handleAllOutputs(const std::vector<int>& Array);
void getInSNMP(const std::vector<int>& Array);
void getOutSNMP(const std::vector<int>& Array);

void printVariableHeader();
void printVariableFooter();

void SNMPsetup(const std::vector<int>& Array)
{
  snmp.setUDP(&udp); // give snmp a pointer to the UDP object
  snmp.begin();      // start the SNMP Manager

  // Get callbacks from creating a handler for each of the OID
  for (const int port : Array) {
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

void snmpLoop(const std::vector<int>& Array){ // the port array, the amount of values in the array, and which array it is: 1st array would be 1, etc.
  getInSNMP(Array);
  getOutSNMP(Array);
  handleAllOutputs(Array);
}

void handleAllOutputs(const std::vector<int>& Array){
  for (const int port : Array) {

    unsigned long long int subT = 0; // to store the difference between response and last response

    // Debugging: Print the values to help identify issues
    // Some data displays as negative when first uploading, but it balances out after you give it a few polls

    // Prints and sets the ports IN data:
    if (SNMPDEBUG){
      Serial.println();
      Serial.printf("Port %i IN: ", port);
    }
    subT = responseInOctets[port] - lastInOctets[port]; // response from port o minus last response from port o
    if(subT>DATA_CAP){subT=DATA_CAP; if(SNMPDEBUG){Serial.print("☆ ");}}
    if (SNMPDEBUG){Serial.print(subT);}
    lastInOctets[port] = responseInOctets[port]; // set the last to the response, after this is where response can be redefined and it wont matter.
    arrINTotals[port] = subT;
  }
  // Just to get a line between IN and OUT
  if (SNMPDEBUG){Serial.println();}

  // Prints and sets the ports OUT data:
  for (const int port : Array) {
    unsigned long long int subT = 0;
    subT = responseOutOctets[port]-lastOutOctets[port];
    if (SNMPDEBUG){
      Serial.println();
      Serial.printf("Port %i OUT: ", port);
    }
    if(subT>DATA_CAP){subT=DATA_CAP; if (SNMPDEBUG){Serial.print("☆ ");}}
    if(SNMPDEBUG){Serial.print(subT);}
    lastOutOctets[port] = responseOutOctets[port];
    arrOUTTotals[port] = subT;
  }
  if(SNMPDEBUG){Serial.println();}
}

void getInSNMP(const std::vector<int>& Array) // This is a lot of stuff I don't understand, so just research snmp coding and you might understand.
{
  for (const int port : Array) {
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

void getOutSNMP(const std::vector<int>& Array) // Again I have no clue, this was part of the sample code in the library I used.
{
  for (const int port : Array) {
    snmpRequest.addOIDPointer(callbackOutOctets[port]);
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
  Serial.printf("Uptime: %lu\n", uptime);
  Serial.println("----------------------");
}

void updateSNMPUptime()
{
  unsigned long currentTime = millis();
  lastUptime = currentTime;
}
