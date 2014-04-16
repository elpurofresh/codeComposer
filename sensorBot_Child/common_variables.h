/*
 * common_variables.h
 *
 *  Created on: Jul 17, 2012
 *      Author: andres
 */

#ifndef COMMON_VARIABLES_H_
#define COMMON_VARIABLES_H_

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define START_BYTE  0x5B			// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D			// Refer to ']' in ASCII Matrix

#define RED_LED		BIT0
#define GREEN_LED	BIT6

#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define TX_Enable	BIT1			// TX Enable on P2.1 (Inter robot comm)
#define RX_Enable	BIT2			// RX Enable on P2.2 (Inter robot comm)

//#define SD_CHECK   	0				// allows the state of the SD card to be checked
#define SD_Enable0	BIT3			// enable line 0, turns on the SD card
#define SD_Enable1	BIT4			// enable line 1, turns on the SD card.
#define SD_Enable2	BIT5			// enable line 3, turns on the SD card.  bits 3, 4, and 5 must be on at the same time

//#define Bot_ID		0				//this is bot 0
//#define net_size	4				//the network contains 4 bots
#define ON  		1
#define OFF			0

#define MAX_Temp_Buffer 27 //102

unsigned char byteRX = '#';
unsigned int sdCardCmdFlag = 0;

char header[] = "ID P HC IL TS-S SFT OX PH TE AM";

int dataLength = 0;
int storeDataFlag = 0;
int pcktStartFlag = 0;
int lineCnter = 0;
int storeDataCnter = 0;

char uartBuffer[MAX_Temp_Buffer];

char uart_Out[30] =  "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"; // debug
int cnterTest1 = 0;									   // debug

int botID = 1;
int is_parent = 2;
int has_Children = 3;
int is_leaf = 4;
int sendFirsttime = 1;

int OX = 43;
int PH = 7;
int TP = 21;
int AB = 40;

#endif /* COMMON_VARIABLES_H_ */
