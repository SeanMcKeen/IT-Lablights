#include "vector"

extern long int arrINTotals[];
extern long int arrOUTTotals[];

void SNMPsetup(std::vector<int> Array);
void snmpLoop(std::vector<int> Array, int arrayIndex);
void printVariableHeader();
void printVariableFooter();
void callLoop();