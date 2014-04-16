#ifndef COMMON_VARIABLES_H_
#define COMMON_VARIABLES_H_

//general definitions
#define RED_LED		BIT0
#define GREEN_LED	BIT6

#define UART_TXD	BIT1        	// TXD on P1.1
#define UART_RXD	BIT2			// RXD on P1.0


//SD card definitions
#define START_BYTE  0x5B			// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D			// Refer to ']' in ASCII Matrix

#define MAX_Temp_Buffer 6          // 256

//General variables
char uartTime[MAX_Temp_Buffer];
char myTimeSlot = 0; 							//main flag.  runs sensorbot mechanisms



int tempBuffCounter = 0;
int storeDataFlag = 0;
int dataLength = 0;
int pcktStartFlag = 0;
int len = 0;


char data[3] = "$$$";				//holds data that is converted form int to char
int time;
char packet[];

int sec = 0, min = 0, hrs = 0;


int lineCnter = 0, storeDataCnter = 0;


#endif /*COMMON_VARIABLES_H_*/
