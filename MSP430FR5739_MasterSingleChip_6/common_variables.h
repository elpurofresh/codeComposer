/*
 * common_variables.h
 *
 *  Created on: Jul 17, 2012
 *      Author: andres
 */

#ifndef COMMON_VARIABLES_H_
#define COMMON_VARIABLES_H_

#define CPU_FREQ	8000000			// 8MHz

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define START_BYTE  0x5B			// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D			// Refer to ']' in ASCII Matrix

#define UART_RXBot	BIT1			// RXD on P2.1
#define UART_TXBot	BIT0        	// TXD on P2.0
#define UART_RXSd	BIT6			// RXD on P2.6
#define UART_TXSd	BIT5        	// TXD on P2.5

#define SD_Enable0	BIT2			// enable P2.2, turns on the SD card

#define ON  		1
#define OFF			0

#define TRUE		1
#define FALSE		0
#define INDEFINED	'\0'

#define MAX_Temp_BufferSD (27 + 1) 	//where "+1" intends to include the last char of the string '0x0'
#define MAX_BufferSize	102			//Maximum buffer size = largest possible data packet

#define MAX_NUM_DIGITS 2			//Maximum number of digits (0-99) for sec, min, hrs, and sensors' values to be TXed/RXed

unsigned char byteRX_SD = '#';
unsigned int sdCardCmdFlag = 0;

char header[] = "bID prnt hChd hSL sFT ss mm hh #R bID ss mm hh OX PH AM TP"; // bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP";

int dataLength = 0;
int storeDataFlag = 0;
int pcktStartFlagBot = 0;
int pcktStartFlagSD = 0;

int lineCnter = 0;
int storeDataCnter = 0;

char uartBuffer[MAX_BufferSize];

char uart_Out[60] =  "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"; // debug
int cnterTest1 = 0;									   // debug

int hasChild = 3;
int hasSeenLeaf = 4;
int sendFirstTime = 1;

char buffToNetwork[28] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@\0";
volatile int sec = 0;
volatile int min = 11;
volatile int hrs = 22;
/*
 * Time slot for every round interval follows: roundInter = 2*n^2*timeSlot
 * roundInter = round interval
 * n = number of robots
 * timeSlot = time slot (seconds)
 */
volatile int timePerInterval = 4;

volatile int numBotsData = 1;
//this initialization may be done separately at the header
volatile int myTimeSlotCounter = 0;
volatile int numNodes = 3;   //number of children plus master node


volatile int myTimeSlotInterval = 0;

volatile int myBotID = 1;
volatile int myParent = INDEFINED;
volatile int myHasChild = FALSE;
volatile int myHasSeenLeaf = FALSE;
volatile int mySndFstTime = -1;
volatile int myNumChildren = 0;
volatile int myChildrenSntDataSuccess = FALSE;

//volatile int packetHasSeenLeaf = 0;
volatile int packetBotId = 0;
//volatile int packetParent = 0;
volatile int packetTimeSlot = 0;

volatile int OX = 43; //43 - 34
volatile int PH = 10; //10 - 01
volatile int TP = 20; //20 - 02
volatile int AB = 66; //66 - 33

/*volatile int OX = 34; //43 - 34
volatile int PH = 01; //10 - 01
volatile int TP = 02; //20 - 02
volatile int AB = 33;*/

volatile char networkData[2][12]; //botID /--/ parent(1), hChild(1), hSnLf(1), sndFstTime(1), timeSlot(1), hh(2), mm(2), ss(2), \0
volatile char robotsData[2][15];  //botID /--/ hh, mm, ss, OX, PH, TP, AB, \0
volatile char myChildrenList[7]; //= "xxxxxx\0";
volatile int rxdFromChildren[6]; // = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

static enum {
	Analyzing,
	Broadcasting,
	Counting,
	Listening,
	Saving
} OperationMode = Listening;




#endif /* COMMON_VARIABLES_H_ */
