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
#include <string.h>

#define LED1		BIT0
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define RESET_PIN	BIT4			// Reset line on P1.4

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define CTRLZ_ASCII	26
#define CR_ASCII	13
#define SPACE_ASCII 32
#define DEL_ASCII	127

#define MAX_BUFF	16				// Maximum buffer size
#define MAX_BigBuffer 256

unsigned char bufferRX;

unsigned char dataVals[MAX_BUFF];
char dataIn[MAX_BUFF] = "$$$$$$$$$$$$$$$$";

char dataTest[] = "t986,ph4,cd205";
char dataTest_1[] = "1@3$5^7*9)";
char uBuff[MAX_BigBuffer];

unsigned int cmdFlag = 0;
int newFileCreatedFlag = 0;

unsigned int task = 9;

int numLines = 0, counter = 0, endReadDataLine = 0, cnter = 0;
int storeDataFlag = 0;
int dataLength = 0;

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

void createNewFile(const char *buf);

void readData(void);
void writeNewSensorData(char *data);
void writeDataToFile(const char *buf, char *data);
void writeData(char *buf);
void readDataFromFile(char *buf);
void cardStatus(void);
void sleep(void);

void parseData(void);

void testSendChars(void){
	UART_putChar('$');
	sleep();
	UART_putChar('1');
	sleep();
	sleep();
	UART_putChar('@');
	UART_putChar(CR);
}

void initBigBuffer(void) {
	int i = 0;
	for (i = 0; i < MAX_BigBuffer; i++) {
		uBuff[i] = '$';
	}
}

void main(void)
{
	setupUART();

	_EINT();

	initBigBuffer();

	turnONSdCard();
	task = 3;

	while(1){

		cardStatus();

		switch (task) {
		case 0:
			sendEscapeCmd();
			task = 1;			// Avoid keep getting in this case statement
			break;

		case 1:
			createNewFile("new data.txt");
			task = 2;
			break;

		case 2:
			writeDataToFile("append data.txt", dataTest);
			task = 9;
			break;

		case 3:
			readDataFromFile("read data.txt");
			task = 4;
			break;

		case 4:
			createNewFile("new data1.txt");
			writeDataToFile("append data1.txt", dataIn);
			task = 9;
			break;

		case 5:
			writeDataToFile("append data.txt", dataTest_1);
			task = 9;
			break;

		default:
			break;
		}

	}

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

void cardStatus(void)
{
	switch (bufferRX) {
	case '<':
		cmdFlag	= 0;
		break;
	case '>':
		cmdFlag	= 1;
		break;
	}

	sleep();
}

void turnONSdCard(void) {
	P2DIR |= BIT0 + BIT1;
	P2OUT |= BIT0 + BIT1;
}

void turnOFFSdCard(void) {
	P2OUT &= ~BIT0 & ~BIT1;
}

void sendEscapeCmd(void) {

	if (cmdFlag == 0) {
		UART_putChar(CTRLZ);
		UART_putChar(CTRLZ);
		UART_putChar(CTRLZ);

	} else {
		UART_putChar(CR);
	}
}

void createNewFile(const char *buf) {
	sendEscapeCmd();
	UART_write(buf);
}

void writeDataToFile(const char *buf, char *data) {  // Assumes target file already exists

	UART_write(buf);      	//append cmd
	writeData(data);		//write actual data
	sleep();
	cmdFlag = 0;
	sendEscapeCmd();		//escape: go back to cmd prompt '>'
}

void readDataFromFile(char *buf) {  // Assumes file to be read already exists
	UART_write(buf);
	storeDataFlag = 1;
}

void parseData(void) {
	int i = 0;
	char temp[32];

	for (i = 0; i < sizeof(uBuff); i++) {    //Count number of EndOfLine markers, i.e. CR
		if (uBuff[i] == '#') {   			 //Checking for our EndOfLine marker
			numLines++;
		}
	}

	for (i = 0; i < sizeof(uBuff); i++) {  	//Count again...
		if (uBuff[i] == '#') {
			counter++;

			if (counter == numLines) {		//...when equal we have in temp the last line
				endReadDataLine = 1;		//don't read anymore from uBuff
			} else {
				dataLength = 0;   			//reset the position of temp array to store a new line
			}

		} else if ( (uBuff[i] <= 32) || (uBuff[i] == 127) || (uBuff[i] == 46)) {
			//skip characters
		} else {
			if (endReadDataLine == 0) {
				temp[dataLength] = uBuff[i];
				dataLength++;
			}
		}
	}

	for (i = 0; i < dataLength; ++i) {  //For the size of the data store only those values in dataIn
		dataIn[i] = temp[i];
	}
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
	//bufferRX = '#';				 //flush the buffer to avoid getting in a endless echo loop
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
	for (i = 0; i < strlen(buf); i++) {
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

void writeNewSensorData(char *data) {
	int i;
	int len = MAX_BUFF;
	for (i = 0; i < len; i++) {
		UART_putChar (data[i]);
	}

	UART_putChar(CR);
}

void writeData(char *buf) {
	int i;
	int len = strlen(buf);
	for (i = 0; i < len; i++) {
		UART_putChar (buf[i]);
	}

	UART_putChar('#');   // EndOfLine marker
	UART_putChar(CR);
}

void sleep(void)
{
	_delay_cycles(1000000);
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

}

/*void waitForCmdPrompt(void) {
	while(1){
		if (bufferRX == '>') {
			cmdFlag = 1;
			break;
		} else {
			cmdFlag = 0;
		}
	}
}*/

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	bufferRX = UCA0RXBUF;                     // Store RXed character

	if (storeDataFlag == 1) {
		if (cnter == MAX_BigBuffer-1) {
			cnter = 0;
		} else {
			uBuff[cnter] = bufferRX;
			cnter++;

			if (bufferRX == '>') {
				storeDataFlag = 0;
				parseData();
			}
		}
	}

}

