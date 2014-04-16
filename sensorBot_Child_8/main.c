/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"
#include "i2C_master.h"
#include <stdio.h>
#include <stdlib.h>

void genPacket(void);
void txPacket(char *buffer);
void parsePacket(void);
void broadCast(void);
void savingToSdCard(void);
void timeKeeper(void);
void resetClock(unsigned char *recvdTime, unsigned char *transTime);
void resetTime(unsigned char *resetTime);
void getTime(unsigned char *getTime);

#define SLAVE_ADDRESS 			0x48
#define PRESCALE_FAC_BAUDRATE	0x12
#define SDA						BIT6	// Port 1.6 UCB0SDA
#define SCL						BIT7	// Port 1.7 UCB0SCL

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

unsigned char timeReset[6] = "000000";
unsigned char timeRX[6] = "@@@@@@";
unsigned char timeTX[6] = "S@@@@@";

int portFlag = 0;

void main(void) {

	initClocks();
	initUARTSdCard();
	initUARTBot();

	initLEDs();
	//initLEDWait();	// Sleep for 5 sec: 5*8MHz.
	//initUartBuffer();

	P1DIR &= ~BIT4;			// P1.4 set to input
	P1REN |= BIT4;			// P1.4 pull-up set on it
	P1OUT &= ~BIT4;			// P1.4 set to Low
	P1IES &= ~BIT4;			// P1.4 Lo/Hi edge interrupt
	P1IE |= BIT4;			// P1.4 interrupt enabled
	P1IFG &= ~BIT4;			// P1.4 IFG cleared

	_EINT();

	/*initCorrect = checkSDCardStatus(); // Enables/Disables the main while loop

	if (initCorrect) {
		StartUpSequence();
	} else {
		blinkLED8(1);
	}*/

	initCorrect = 1;

	//resetClock(time);

	//OperationMode = Listening;


	while(initCorrect){

		if (portFlag == 1) {
			//resetTime(timeReset);
			getTime(timeRX);
			portFlag = 0;
		}


		/*switch (OperationMode) {

		case Broadcasting:
			broadCast();
			OperationMode = Listening;
			break;

		case Counting:
			break;

		case GetTime:
			requestTime(time);
			break;


		case Listening:
			__bis_SR_register(LPM3_bits + GIE);     // Enter LPM4 w/interrupt
			timeKeeper();
			break;

		case Saving:
			savingToSdCard();
			break;

		default:
			OperationMode = Listening;
			break;
		}*/
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
	genPacket();
	txPacket(buff);

	OperationMode = Listening;
}

void savingToSdCard(void)			// Saves received data on to SD card and operates on the packet
{
	turnSdCard(ON);
	writeDataToSDFile("append myData.txt", uartBuffer, 1);
	turnSdCard(OFF);

	//parsePacket();

	OperationMode = Listening;
}

void timeKeeper(void)
{
	if (sec == 59) { sec = 0; min ++; }

	/*if ( (sec % 3) == 0 ) {
		OperationMode = Listening;			// Set flag to operate on received data package timeToSend = 1;
	}*/

	if ( (sec % 8) == 0 ) {
		OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }
}

void resetClock(unsigned char *recvdTime, unsigned char *transTime)
{
	transmitI2C_init(SLAVE_ADDRESS);	// init transmitting with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	// initialization
	receiveI2C_init(SLAVE_ADDRESS);		// init receiving with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	receiveI2C(6, recvdTime);
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free

	transmitI2C_init(SLAVE_ADDRESS);	// init transmitting with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	transmitI2C(6, transTime);			// start transmitting
}

void resetTime(unsigned char *newTime)
{
	transmitI2C_init(SLAVE_ADDRESS);	// init transmitting with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	// initialization
	transmitI2C(6, newTime);			// start transmitting
}

void getTime(unsigned char *rxTime)
{
	transmitI2C_init(SLAVE_ADDRESS);	// init transmitting with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	transmitI2C(6, timeTX);				// start transmitting

	while ( TI_USCI_I2C_notready() );	// wait for bus to be free

	// initialization
	receiveI2C_init(SLAVE_ADDRESS);		// init receiving with USCI
	while ( TI_USCI_I2C_notready() );	// wait for bus to be free
	receiveI2C(6, rxTime);
}

// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	P1IE &= ~BIT4;
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

	case 4:break;                            // Vector 4 - TXIFG
	default: break;
	}
	P1IE |= BIT4;

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

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	blinkLED4(1);

	portFlag = 1;
	//OperationMode = GetTime;
	P1IFG &= ~BIT4;                         // Clear P1.4 IFG
}

