#ifndef COMMON_VARIABLES_H_
#define COMMON_VARIABLES_H_

//general definitions
#define RED_LED		BIT0
#define GREEN_LED	BIT6
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define SD_Enable0	BIT3			// enable line 0, turns on the SD card
#define SD_Enable1	BIT4			// enable line 1, turns on the SD card.
#define SD_Enable2	BIT5			// enable line 3, turns on the SD card.  bits 3, 4, and 5 must be on at the same time
#define CLK_Enable	BIT0			//closes clock switch
#define	DAC_Enable	BIT7			//closes DAC switch
#define TxRx_Enable	BIT1			//enables/closes inter bot tx/rx switch
#define SD_CHECK   	0				//allows the state of the SD card to be checked
#define CLK_CHECK 	1				//allows the state of the Clock card to be checked
#define DAC_CHECK	2				//allows the state of the DAC card to be checked
#define ts			0				//selects time stamp option in the load packet function
#define sec			1				//selects seconds option in the load packet function
#define ox			3				//selects oxygen option in the load packet function
#define ph			4				//selects PH option in the load packet function
#define te			5				//selects temperature option in the load packet function
#define am	 		6				//selects ambiance option in the load packet function

//SD card definitions
#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define CTRLZ_ASCII	26
#define CR_ASCII	13
#define SPACE_ASCII 32
#define DEL_ASCII	127
#define START_BYTE  0x5B			// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D			// Refer to ']' in ASCII Matrix

#define MAX_Data_Buffer	16			// Maximum buffer size
#define MAX_Temp_Buffer 90          // 256

//General variables
char dataIn[MAX_Temp_Buffer] = "$$$$$$$$$$$$$$$$";
char dataOut[MAX_Temp_Buffer] = "$$$$$$$$$$$$$$$$";
char uartBuffer[MAX_Temp_Buffer];
char myTimeSlot = 0; 							//main flag.  runs sensorbot mechanisms
char processStoreDataFinish = 0;

unsigned char byteRX = '#';

unsigned int sdCardCmdFlag = 0;
unsigned int task = 9;

int tempBuffCounter = 0;
int storeDataFlag = 0;
int dataLength = 0;
int pcktStartFlag = 0;
int len = 0;

//initialization data -- all but bot ID will be gathered from the network.  some of these data types may need to be modified
int botID = 1;
int is_parent = 2;
int has_Children = 3;
int is_leaf = 4;
char sendFirsttime = '7';

int PH = 0;
int OX = 0;
int temp = 0;
int amb = 0;

//in order to reach 720 seconds (12 min) the seconds packet must be broken down into 3 bytes
int sec_0 = 0;
int sec_1 = 0;
int sec_2 = 0;
int timeSlot = 0;				//current time slot
long int secStamp = 0;			//number of seconds that have passed since the start of the time slot

//header of the file on the SD card
char header[] = "ID P HC IL TS-S SFT OX PH TE AM";

volatile unsigned int i;			//general purpose counter


char data[3] = "$$$";				//holds data that is converted form int to char
int time;
char packet[];



int lineCnter = 0, storeDataCnter = 0;

//clock variables


#endif /*COMMON_VARIABLES_H_*/
