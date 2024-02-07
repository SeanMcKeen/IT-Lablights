extern long int arrINTotals[];
extern long int arrOUTTotals[];

void SNMPsetup(int Array[], int sizeOfArray);
void snmpLoop(int Array[], int arrayCount, int arrayIndex);
void printVariableHeader();
void printVariableFooter();
void callLoop();