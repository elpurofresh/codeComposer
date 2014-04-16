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
#include "msp430g2553.h"

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define LED1		BIT0
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define RESET_PIN	BIT4			// Reset line on P1.4

#define MAX_BUFF	16				// Maximum buffer size

unsigned int enterCmdModeSent = 0;
unsigned int newFileCmdSent = 0;
unsigned int apndDataCmdSent = 0;
unsigned int readDataCmdSent = 0;

unsigned char bufferRX;

unsigned char dataVals[MAX_BUFF];

unsigned int writeProcess;// = 0;
unsigned int writeNew;// = 0; //for testing only

unsigned int errorFlag = 0;
unsigned int logFlag = 0;
unsigned int cmdFlag = 0;

void setupUART(void);
void sendEscapeCmd(void);
void blinkLED1(int numOfBlinks);
void blinkLED2(int numOfBlinks);
void blinkLED1n2(int numOfBlinks);
void UART_putChar (char c);
void UART_write (const char *buf);
void writeSensorData(void);
void turnONSdCard(void);
void turnOFFSdCard(void);

//for testing only
void readData(void);
void writeNewSensorData(unsigned char *buf);

void cardStatus(void);

void main(void)
{
	setupUART();

	writeProcess = 1;
	writeNew = 0;

	//int k = 0;
	//int max = 3;
	_EINT();

	turnONSdCard();

	while(1){

		/*if (writeProcess) {
			if (bufferRX == '<') {
				if (sendEscapeCmd()) {
					blinkLED2(3);
				} else {
					blinkLED1(3);
				}
			}

		}*/

		if (writeProcess == 1) {

			if (writeNew == 0) {
				if (bufferRX == '<' && apndDataCmdSent == 0) {
					sendEscapeCmd();
					enterCmdModeSent = 1;
				}

				if (bufferRX == '>' && enterCmdModeSent == 1) {
					UART_write("new DATA100.txt");
					newFileCmdSent = 1;
					enterCmdModeSent = 0;
				}

				if (bufferRX == '>' && newFileCmdSent == 1) {
					UART_write("append DATA100.txt");
					apndDataCmdSent = 1;
					newFileCmdSent = 0;
				}

				if (bufferRX == '<' && apndDataCmdSent == 1) {
					writeSensorData();
					sendEscapeCmd();
					apndDataCmdSent = 0;
					writeProcess = 0;     //Finished writing
				}

			} else if (writeNew == 1) {// After have entered once cmd mode, '>' will become the default prompt
				//if (bufferRX == '>' && apndDataCmdSent == 0) {
				//sendCtrlZCmd();
				enterCmdModeSent = 1;
				//}

				if (bufferRX == '>' && enterCmdModeSent == 1) {
					UART_write("new DATA200.txt");
					newFileCmdSent = 1;
					enterCmdModeSent = 0;
				}

				if (bufferRX == '>' && newFileCmdSent == 1) {
					UART_write("append DATA200.txt");
					apndDataCmdSent = 1;
					newFileCmdSent = 0;
				}

				if (bufferRX == '<' && apndDataCmdSent == 1) {
					writeNewSensorData(dataVals);
					sendEscapeCmd();
					apndDataCmdSent = 0;
					writeProcess = 99;     //Finished writing don't get in a deadlock
				}
			}

		} else if (writeProcess == 0)  {
			blinkLED2(1);

			if (bufferRX == '>' && readDataCmdSent == 0) {
				sendEscapeCmd();
				readDataCmdSent = 1;
			}

			if (bufferRX == '>' && readDataCmdSent == 1) {
				readData();
				readDataCmdSent = 0;
				writeProcess = 1;  //Enable Writing again
				writeNew = 1;
			}

		} else if (writeProcess == 99) {
			turnOFFSdCard();
		}
	}
}

void cardStatus(void)
{
	switch (bufferRX) {
	case '!':
		errorFlag = 1;
		logFlag = 0;
		cmdFlag	= 0;
		blinkLED1(2);
		break;
	case '<':
		errorFlag = 0;
		logFlag = 1;
		cmdFlag	= 0;
		blinkLED2(2);
		break;
	case '>':
		errorFlag = 0;
		logFlag = 0;
		cmdFlag	= 1;
		blinkLED1n2(2);
		break;
		//default:
		//	break;
	}
}

void turnONSdCard(void) {
	P2DIR |= BIT0 + BIT1;
	P2OUT |= BIT0 + BIT1;
	blinkLED1(1);
}

void turnOFFSdCard(void) {
	P2OUT &= ~BIT0 & ~BIT1;
	blinkLED2(1);
}

void setupUART(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
	P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                // All P1.x reset
	P1SEL = UART_RXD + UART_TXD;              // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD + UART_TXD;             // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}


void sendEscapeCmd(void)
{
	int i;
	int numCmdsToSend = 3;
	for (i = 0; i < numCmdsToSend; ++i) {
		UART_putChar(CTRLZ);
	}
	if (apndDataCmdSent) { //If in appending mode
		UART_putChar(CR); //after exiting once, continuing sending this causes "unknown command" error msg from SD card
	}


	/*if (bufferRX == '>') {
		return 1;   //Success
	} else {
		return 0;   //Failure
	}*/
}

void blinkLED1(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; i++) {
		P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
	//P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}

void blinkLED2(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= BIT6;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
	//P1OUT &= ~BIT6;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}

void blinkLED1n2(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
		P1OUT ^= BIT6;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}

	//P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
	//P1OUT &= ~BIT6;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}

/**
 * Send only byte via UART.
 *
 * @param c Character to send.
 */
void UART_putChar (char charToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data
	bufferRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}

/**
 * Write string buffer to UART.
 *
 * @param buf String buffer to send.
 * @param length Length of string buffer.
 */
void UART_write (const char *buf)
{
	int i;
	//int len = sizeof(buf);
	for (i = 0; i < strlen(buf); i++) { //strlen(buf)
		UART_putChar (buf[i]);
	}

	UART_putChar(CR);
}

void writeSensorData(void)
{
	UART_write("t98.6,ph4,cd20.5");
	UART_putChar(CR);
}

void readData(void) {
	int i = 0;
	UART_write("read DATA100.txt");
	while (bufferRX != CR /*|| bufferRX != '#'*/){
		dataVals[i] = bufferRX;
		i++;
	}

	blinkLED1(3);
}

void writeNewSensorData(unsigned char *buf) {
	int i;
	int len = MAX_BUFF;
	for (i = 0; i < len; i++) {
		UART_putChar (buf[i]);
	}

	UART_putChar(CR);
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	//UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character  Echoes received byte
	bufferRX = UCA0RXBUF;                     // Store RXed character
}

