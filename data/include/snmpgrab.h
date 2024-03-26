#include <vector>
extern int arrTotals[4][2];

void SNMPsetup(const std::vector<int>& ports);
void snmpLoop(const std::vector<int>& ports);
void printVariableHeader();
void printVariableFooter();
void callLoop();
bool isConnectedToSwitch();