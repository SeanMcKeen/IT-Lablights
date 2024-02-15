#include "vector"

extern unsigned long long int arrINTotals[];
extern unsigned long long int arrOUTTotals[];
extern unsigned int lastUptime;

void SNMPsetup(const std::vector<int>& Array);
void snmpLoop(const std::vector<int>& Array);
void printVariableHeader();
void updateSNMPUptime();
void callLoop();