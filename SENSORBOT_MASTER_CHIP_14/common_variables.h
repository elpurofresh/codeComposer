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
#define SD_CHECK   	0				//allows the state of the SD card to be checked
#define CLK_CHECK 	1				//allows the state of the Clock card to be checked
#define DAC_CHECK	2				//allows the state of the DAC card to be checked

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
char dataOut[MAX_Data_Buffer] = "$$$$$$$$$$$$$$$$";
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
int counterTemp = 0;							//used for testing only--temporary counter used to catch all incoming data


int lineCnter = 0, storeDataCnter = 0;

//clock variables


#endif /*COMMON_VARIABLES_H_*/
