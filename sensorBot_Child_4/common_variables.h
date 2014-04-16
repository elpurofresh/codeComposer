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

#define UART_RXBot	BIT1			// RXD on P2.1
#define UART_TXBot	BIT0        	// TXD on P2.0
#define UART_RXSd	BIT6			// RXD on P2.6
#define UART_TXSd	BIT5        	// TXD on P2.5

#define SD_Enable0	BIT2			// enable P2.2, turns on the SD card

#define ON  		1
#define OFF			0

#define MAX_Temp_BufferSD (27 + 1) 	//where "+1" intends to include the last char of the string '0x0'
#define MAX_BufferSize	102			//Maximum buffer size = largest possible data packet

unsigned char byteRX_SD = '#';
unsigned int sdCardCmdFlag = 0;

char header[] = "bID prnt hChdn hSL ss mm hh sFT #R bID ss mm hh OX PH AM TP"; // bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP bID ss mm hh OX PH AM TP";

int dataLength = 0;
int storeDataFlag = 0;
int pcktStartFlag = 0;
int lineCnter = 0;
int storeDataCnter = 0;

char uartBuffer[MAX_BufferSize];

char uart_Out[60] =  "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"; // debug
int cnterTest1 = 0;									   // debug

int botID = 1;
int is_parent = 2;
int has_Children = 3;
int is_leaf = 4;
int sendFirstTime = 1;

int OX = 34;
int PH = 7;
int TP = 12;
int AB = 40;

static enum {
	Broadcasting,
	Receiving,
	Sleeping
} OperationMode = Sleeping;

#endif /* COMMON_VARIABLES_H_ */
