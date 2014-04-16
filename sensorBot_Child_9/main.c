/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"
//#include "i2C_master.h"
#include <stdio.h>
#include <stdlib.h>

void genPacket(void);
void txPacket(char *buffer);
void parsePacket(void);
void broadCast(void);
void savingToSdCard(void);
void timeKeeper(void);

int sec = 0;
int min = 0;
int hrs = 0;
int numBotsData = 0;

int initCorrect = 0;

int cnterTest = 0;
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
char buff[28] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
int buffLen = 0;

char tsec1[2] = "$$", tmin1[2] = "$$", thrs1[2] = "$$", tPH1[2] = "$$";
int tsec2 = 0, tmin2 = 0, thrs2 = 0, tPH2 = 0;

int interBotFlag = 0;

void main(void) {

	initClocks();
	initUARTSdCard();
	initUARTBot();

	initLEDs();
	//initLEDWait();	// Sleep for 5 sec: 5*8MHz.
	//initUartBuffer();

	_EINT();

	/*initCorrect = checkSDCardStatus(); // Enables/Disables the main while loop

	if (initCorrect) {
		StartUpSequence();
	} else {
		blinkLED8(1);
	}*/

	initCorrect = 1;

	//OperationMode = Listening;

	setupTimer();

	//UCA0IE &= ~UCRXIE;                         	// Disable RX interrupt

	while(initCorrect){


		_bis_SR_register(LPM3_bits + GIE);

		timeKeeper();

		if (interBotFlag == 0) {
			TA0CCTL0 |= CCIE;
			UCA0IE |= UCRXIE;				 // Enable further reception of data
		}


		/*switch (OperationMode) {

		case Broadcasting:
			broadCast();
			break;

		case Listening:
			break;

		case Saving:
			savingToSdCard();
			break;

		default:
			OperationMode = Listening;
			break;
		}*/

		//TA0CCTL0 |= CCIE;

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
	int i = 0, j = 0;

	for (i = 0; i < 5; ++i) {

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

	sprintf(tPH1, "%c%c", uartBuffer[20], uartBuffer[21]);  	// this will not work after more than one robot
	tPH2 = atoi(tPH1);
	tPH2 = tPH2 - 14;
}

void broadCast(void)
{
	PJOUT ^= BIT2;

	UCA0IE &= ~UCRXIE;

	genPacket();
	txPacket(buff);

	//UCA0IE |= UCRXIE;
	interBotFlag = 0;

	//OperationMode = Listening;
	//UCA0IE |= UCRXIE;				 // Enable further reception of data
	//TA0CCTL0 |= CCIE;

	PJOUT ^= BIT2;
}

void savingToSdCard(void)			// Saves received data on to SD card and operates on the packet
{
	turnSdCard(ON);
	writeDataToSDFile("append myData.txt", uartBuffer, 1);
	turnSdCard(OFF);

	//parsePacket();

	//UCA0IE |= UCRXIE;				 // Enable further reception of data

	interBotFlag = 0;

	//OperationMode = Listening;
}

void timeKeeper(void)
{
	PJOUT ^= BIT1;
	if (sec == 59) { sec = 0; min ++; }

	if ( (sec % 14) == 0 ) {
		//OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
		broadCast();
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }
	PJOUT ^= BIT1;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	PJOUT ^= BIT3;
	sec++;

	TA0CCTL0 &= ~CCIE;

	PJOUT ^= BIT3;

	__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return
}

// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))	  // Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		interBotFlag = 1;

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

			UCA0IE &= ~UCRXIE;				 // Disable further reception of data
			//OperationMode = Saving;		 //Set flag to operate on received data package

			savingToSdCard();
		}

		P3OUT ^= BIT6;
		break;

	case 4:break;                            // Vector 4 - TXIFG
	default: break;
	}

	__bic_SR_register_on_exit(LPM3_bits + GIE);
}

// Receives data from the SD card
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,0x08))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		while (!(UCTXIFG & UCA1IFG));           // USCI_A1 TX buffer ready?

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
	case 4:break;                             // Vector 4 - TXIFG
	default: break;
	}

	__bic_SR_register_on_exit(LPM3_bits + GIE);
}

