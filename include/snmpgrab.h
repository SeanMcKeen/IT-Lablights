extern int arrTotals[4][2];

void SNMPsetup(int Array[], int sizeOfArray);
void snmpLoop(int Array[], int arrayCount, int arrayIndex);
void printVariableHeader();
void printVariableFooter();
void callLoop();