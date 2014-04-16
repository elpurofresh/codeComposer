/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"
//#include <stdio.h>
//#include <stdlib.h>

void genPacket(void);
void txPacket(char *buffer);
void parsePacket(void);
void broadCast(void);
void savingToSdCard(void);

void resetClock(void);
void timeKeeper(void);

volatile int sec = 0;
volatile int min = 11;
volatile int hrs = 22;
volatile int numBotsData = 1;

volatile int initCorrect = 0;

volatile int cnterTest = 0;
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
char buff[27] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@";
volatile int buffLen = 0;

char tsec1[2] = "$$", tmin1[2] = "$$", thrs1[2] = "$$", tPH1[2] = "$$";
int tsec2 = 0, tmin2 = 0, thrs2 = 0, tPH2 = 0;

volatile int startCounting = 0;

void main(void) {


	initClocks();
	initUARTSdCard();
	initUARTBot();

	initLEDs();
	initUartBuffer();

	setupPorts();

	_EINT();

	initCorrect = 1;

	OperationMode = Listening;
	startExternalClock();

	while(initCorrect){

		switch (OperationMode) {

		case Analyzing:
			//parsePacket();
			resetClock();
			OperationMode = Listening;
			break;

		case Broadcasting:
			stopRXBots();
			stopExternalClock();
			broadCast();
			startExternalClock();

			OperationMode = Listening;
			break;

		case Listening:
			startRXBots();
			__low_power_mode_3();
			break;

		case Saving:
			stopRXBots();
			stopExternalClock();

			savingToSdCard();

			startExternalClock();

			//OperationMode = Listening;
			OperationMode = Analyzing;
			break;

		default:
			OperationMode = Listening;
			break;
		}

		timeKeeper();

		//__low_power_mode_3();  // <-this should go up to listening

	}
}

void genPacket(void)
{
	char temp1[2] = "0\0";
	char temp2[3] = "00\0";

	itoa(botID, temp1);
	buff[0] = temp1[0];

	itoa(is_parent, temp1);
	buff[1] = temp1[0];

	itoa(has_Children, temp1);
	buff[2] = temp1[0];

	itoa(is_leaf, temp1);
	buff[3] = temp1[0];

	itoa(sendFirstTime, temp1);
	buff[4] = temp1[0];

	itoa(sec, temp2);
	buff[5] = temp2[0];
	buff[6] = temp2[1];
	temp2[0] = '0'; temp2[1] = '0';

	itoa(min, temp2);
	buff[7] = temp2[0];
	buff[8] = temp2[1];
	temp2[0] = '0';	temp2[1] = '0';

	itoa(hrs, temp2);
	buff[9] = temp2[0];
	buff[10] = temp2[1];

	itoa(numBotsData, temp1);
	buff[11] = temp1[0];

	itoa(botID, temp1);
	buff[12] = temp1[0];

	itoa(sec, temp2);
	buff[13] = temp2[0];
	buff[14] = temp2[1];

	itoa(min, temp2);
	buff[15] = temp2[0];
	buff[16] = temp2[1];

	itoa(hrs, temp2);
	buff[17] = temp2[0];
	buff[18] = temp2[1];

	itoa(OX, temp2);
	buff[19] = temp2[0];
	buff[20] = temp2[1];

	itoa(PH, temp2);
	buff[21] = temp2[0];
	buff[22] = temp2[1];

	itoa(TP, temp2);
	buff[23] = temp2[0];
	buff[24] = temp2[1];

	itoa(AB, temp2);
	buff[25] = temp2[0];
	buff[26] = temp2[1];
}

void txPacket(char *buffer)
{
	static int i = 0, j = 0;

	for (i = 0; i < 10; ++i) {

		//P3OUT ^= BIT7;
		UART_BotputChar(START_BYTE);

		for (j = 0; j < strlen(buffer); ++j) {  // Send 10 times the data packet with a 0.25sec pause in between
			UART_BotputChar(buffer[j]);
		}
		UART_BotputChar(END_BYTE);
		//P3OUT ^= BIT7;

		sleepQuart();
	}
}

void parsePacket(void)
{
	/*sprintf(tPH1, "%c%c", uartBuffer[20], uartBuffer[21]);  	// this will not work after more than one robot
	tPH2 = atoi(tPH1);
	tPH2 = tPH2 - 14;*/

}

void broadCast(void)
{
	//stopTimer();
	genPacket();
	txPacket(buff);
	//startTimer();
}

void savingToSdCard(void)			// Saves received data on to SD card and operates on the packet
{
	turnSdCard(ON);
	writeDataToSDFile("append myData.txt", uartBuffer, 1);
	turnSdCard(OFF);
}

void resetClock(void)
{
	char temp[3] = "@@\0";
	temp[0] = uartBuffer[5];
	temp[1] = uartBuffer[6];
	sec = atoi(temp);

	temp[0] = uartBuffer[7];
	temp[1] = uartBuffer[8];
	min = atoi(temp);

	temp[0] = uartBuffer[9];
	temp[1] = uartBuffer[10];
	hrs = atoi(temp);
}

void timeKeeper(void)
{
	if (sec == 59) { sec = 0; min ++; }
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }

	if ( (startCounting == 1) && ((sec % 10) == 0) ) {
		OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
	}
}



// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))	  // Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		P3OUT ^= BIT6;
		if (!pcktStartFlagBot && UCA0RXBUF == START_BYTE) {
			pcktStartFlagBot = 1;
			dataLength = 0;					  //Reset initial position in buffer (if no END_BYTE is seen at the end of package error will occur
		}
		else if (pcktStartFlagBot && UCA0RXBUF != END_BYTE) {
			uartBuffer[dataLength] = UCA0RXBUF;
			dataLength++;
		}
		else if (pcktStartFlagBot && UCA0RXBUF == END_BYTE) {
			pcktStartFlagBot = 0;			 //Reset framing

			OperationMode = Saving;			 //Set flag to operate on received data package
		}
		P3OUT ^= BIT6;
		break;

	default: break;
	}

	__low_power_mode_off_on_exit();
}

// Receives data from the SD card
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,0x02))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		byteRX_SD = UCA1RXBUF;                     // Store RXed character

		uart_Test[cnterTest] = byteRX_SD;

		if (!pcktStartFlagSD && byteRX_SD == START_BYTE) {
			pcktStartFlagSD = 1;
			dataLength = 0;						// Reset initial position in buffer (if no END_BYTE is seen at the end of package error will occur
		}
		else if (pcktStartFlagSD && byteRX_SD != END_BYTE) {
			uartBuffer[dataLength] = byteRX_SD;
			dataLength++;
		}
		else if (pcktStartFlagSD && byteRX_SD == END_BYTE) {
			pcktStartFlagSD = 0;					// Reset framing
			lineCnter++;						// Increase number of lines
		}

		cnterTest++;
		if (cnterTest > 60) {
			cnterTest = 0;
		}

		break;

	default: break;
	}

	__low_power_mode_off_on_exit();
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	PJOUT |= BIT0;

	__delay_cycles(80);
	sec++;
	P1IFG &= ~BIT4;				// Clear P1.4 IFG
	startCounting = 1;			// Avoids transmitting at seconds=0

	PJOUT &= ~BIT0;

	__low_power_mode_off_on_exit();
}

