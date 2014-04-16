/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"
#include <stdio.h>
#include <stdlib.h>

void genPacket(void);
void txPacket(char *buffer);
void parsePacket(void);
void broadCast(void);
void savingToSdCard(void);

void resetClock(void);
void timeKeeper(void);

void startTimer(void);
void stopTimer(void);

void startRXBots(void);
void stopRXBots(void);

void setupPorts(void);

void startExternalClock(void);
void stopExternalClock(void);

volatile int sec = 0;
volatile int min = 0;
volatile int hrs = 0;
volatile int numBotsData = 0;

volatile int initCorrect = 0;

volatile int cnterTest = 0;
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
char buff[28] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
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
			parsePacket();
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
			break;

		case Saving:
			stopRXBots();
			stopExternalClock();

			savingToSdCard();

			startExternalClock();

			OperationMode = Listening;
			//OperationMode = Analyzing;
			break;

		default:
			OperationMode = Listening;
			break;
		}

		timeKeeper();

		__low_power_mode_3();

	}
}

void genPacket(void)
{
	int tsec = 0, tmin = 0, thrs = 0, tPH = 0;
	numBotsData = 1;

	if ((sec - 9) <= 0) {
		tsec = sec + 60;
	} else {
		tsec = sec;
	}

	if ((min - 9) <= 0) {
		tmin = min + 60;
	} else {
		tmin = min;
	}

	thrs = hrs + 24;

	tPH = PH + 14;

	sprintf(buff, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", botID, is_parent, has_Children, is_leaf, tsec, tmin, thrs, sendFirstTime, numBotsData, botID, tsec, tmin, thrs, OX, tPH, TP, AB);
}

void txPacket(char *buffer)
{
	static int i = 0, j = 0;

	for (i = 0; i < 10; ++i) {

		P3OUT ^= BIT7;
		UART_BotputChar(START_BYTE);

		for (j = 0; j < strlen(buffer); ++j) {  // Send 10 times the data packet with a 0.25sec pause in between
			UART_BotputChar(buffer[j]);
		}
		UART_BotputChar(END_BYTE);
		P3OUT ^= BIT7;

		sleepQuart();
	}
}

void parsePacket(void)
{
	sprintf(tPH1, "%c%c", uartBuffer[20], uartBuffer[21]);  	// this will not work after more than one robot
	tPH2 = atoi(tPH1);
	tPH2 = tPH2 - 14;
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
	sprintf(tsec1, "%c%c", uartBuffer[4], uartBuffer[5]);		// seconds of the network
	tsec2 = atoi(tsec1);
	if (tsec2 >= 60) {
		tsec2 = tsec2 - 60;
	}
	sec = tsec2;

	sprintf(tmin1, "%c%c", uartBuffer[6], uartBuffer[7]);		// minutes of the network
	tmin2 = atoi(tmin1);
	if (tmin2 >= 60) {
		tmin2 = tmin2 - 60;
	}
	min = tmin2;

	sprintf(thrs1, "%c%c", uartBuffer[8], uartBuffer[9]);		// hours of the network
	thrs2 = atoi(thrs1);
	hrs = thrs2 - 24;
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

void startTimer(void)
{
	P1IE |= BIT4;
	startExternalClock();
}

void stopTimer(void)
{
	P1IE &= ~BIT4;
	stopExternalClock();
}

void startRXBots(void)
{
	UCA0IE |= UCRXIE;
}

void stopRXBots(void)
{
	UCA0IE &= ~UCRXIE;
}

void startExternalClock(void)
{
	P1OUT |= BIT5;
}

void stopExternalClock(void)
{
	P1OUT &= ~BIT5;
}

void setupPorts(void)
{
	P1DIR |= 0xFF;
	P1OUT =  0;
	//P1DIR |= (BIT0 + BIT5);
	//P1OUT &= ~(BIT0 + BIT5);

	P1DIR &= ~BIT4;		// P1.4 set to ouput
	P1REN |= BIT4;		// P1.4 pull-down set on it
	P1OUT &= ~BIT4;
	P1IN  |= BIT4;		// P1.4 set to Low
	P1IES &= ~BIT4;		// P1.4 Lo/Hi edge interrupt
	P1IE  |= BIT4;		// P1.4 interrupt enabled
	P1IFG &= ~BIT4;		// P1.4 IFG cleared

	PJDIR |= (BIT0 + BIT1 + BIT2 + BIT3);
	PJOUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);

	P3DIR |= (BIT4 + BIT5 + BIT6 + BIT7);
	P3OUT &= ~(BIT4 + BIT5 + BIT6 + BIT7);
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

