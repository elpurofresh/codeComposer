//slave
//******************************************************************************
//   MSP430G2xx3 Demo - USCI_A0, Ultra-Low Pwr UART 9600 String, 32kHz ACLK
//
//   Description: This program demonstrates a full-duplex 9600-baud UART using
//   USCI_A0 and a 32kHz crystal.  The program will wait in LPM0, and will
//   respond to a received 'u' character using 8N1 protocol. The response will
//   be the string 'Hello World'.
//   ACLK = BRCLK = LFXT1 = 32768Hz, MCLK = SMCLK = DCO ~1.2MHz
//   Baud rate divider with 32768Hz XTAL @9600 = 32768Hz/9600 = 3.41
//* An external watch crystal is required on XIN XOUT for ACLK *//
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 | 32kHz
//          --|RST          XOUT|-
//            |                 |
//            |     P1.2/UCA0TXD|------------>
//            |                 | 9600 - 8N1
//            |     P1.1/UCA0RXD|<------------
//
//   D. Dang
//   Texas Instruments Inc.
//   February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include	"msp430g2553.h"
#include	"common_functions.h"
#include	"common_variables.h"
#include 	<stdio.h>


int timeToSend = 0;
int sec = 0;
int min = 0;
int hrs = 0;
int numBotsData = 0;

int cnt1 = 0;
int cnt2 = 0;
int cnt3 = 0;
int cnt4 = 0;
int initCorrect = 0;

int cnterTest = 0;
//char uart_Test[30] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";

char buff[28] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@"; //we may need to include another space in the array to accommodate the '0' character

//void test(char a, int b);
void genPacket(void);
void txPacket(char *buff);

void main(void)
{

	configClocks();
	setupUART();
	setupOutputs();
	setupTimer();

	_EINT();

	//initCorrect = checkSDCardStatus(); //Enables/Disables the main while loop

	initCorrect = 1;

	while(initCorrect)	{
		cnt1++;

		if (timeToSend == 1) {
			cnt2++;
			blinkLEDGreen(2);
			genPacket();

			txPacket(buff);
			storeDataFlag = 1;

			//sec = sec + 5;
			//min = min + 2;
			//hrs = hrs + 8;
			timeToSend = 0;

		} else {
			cnt3++;

			// Received Data if detected
			__bis_SR_register(LPM3_bits + GIE); // Enter LPM3 w/interrupts. This will not affect the counter.
		}
	}
}

/*void test(char a, int b) {
	 sprintf(buff, "%c minus %d",a,b);
}*/

void genPacket(void)
{
	int tsec = 0, tmin = 0, thrs = 0, tPH = 0;
	numBotsData = 1;

	if ((sec - 9) <= 0) {
		tsec = sec + 60;
		cnt4++;
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

	sprintf(buff, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", botID, is_parent, has_Children, is_leaf, tsec, tmin, thrs, sendFirsttime, numBotsData, botID, tsec, tmin, thrs, OX, tPH, TP, AB);

}

void txPacket(char *buff)
{
	int i = 0;
	turnTXD(ON);
	for (i = 0; i < 20; ++i) {  // Send 20 times the data packet with a 0.25sec pause in between
		writeData(buff, 0);
		sleepQuart();
	}
	turnTXD(OFF);
}



//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     // Store RXed character

	uart_Test[cnterTest] = byteRX;

	if (storeDataFlag == 1) {

		if (dataLength == MAX_Temp_Buffer-1) {
			dataLength = 0;
		}

		if (!pcktStartFlag && byteRX == START_BYTE) {
			pcktStartFlag = 1;
			dataLength = 0;
		}
		else if (pcktStartFlag && byteRX != END_BYTE) {
			uartBuffer[dataLength] = byteRX;
			dataLength++;
		}
		else if (pcktStartFlag && byteRX == END_BYTE) {
			pcktStartFlag = 0;
			lineCnter++;
			//endRxProcess = 1;
		}

		else if (!pcktStartFlag && byteRX == '>') {
			//storeDataInVar();
			storeDataFlag = 0;
		}
	}

	cnterTest++;
	if (cnterTest > 30) {
		cnterTest = 0;
	}

	//__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	sec++;
	if (sec == 59) { sec = 0; min ++; }
	if ((sec % 5) == 0) { timeToSend = 1;	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }

	blinkLEDRed(1);
	__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return
}

