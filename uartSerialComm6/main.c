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
#define START_BYTE  0x5B			// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D			// Refer to ']' in ASCII Matrix

#define MAX_Data_Buffer	16				// Maximum buffer size
#define MAX_Temp_Buffer 256

char dataIn[MAX_Data_Buffer] = "$$$$$$$$$$$$$$$$";
char dataTest[] = "t9.86,ph4,cd20^5";
char dataTest_1[] = "1@3$5^7*9)";
char uartBuffer[MAX_Temp_Buffer];

unsigned char byteRX;

unsigned int cmdFlag = 0;
unsigned int task = 9;

int tempBuffCounter = 0;
int storeDataFlag = 0;
int dataLength = 0;
int pcktStartFlag = 0;
int len = 0;
char processStoreDataFinish = 0;

int lineCnter = 0, storeDataCnter = 0;

void setupUART(void);

void blinkLED1(int numOfBlinks);
void blinkLED2(int numOfBlinks);
void blinkLED1n2(int numOfBlinks);

void turnONSdCard(void);
void turnOFFSdCard(void);
void cardStatus(void);
void sleep(void);

void initUartBuffer(void);

void UART_putChar (char c);
void UART_write (const char *buf);

void sendEscapeCmd(void);
void createNewFile(const char *buf);
void writeDataToFile(const char *buf, char *data, int varOrFixed);
void writeData(char *buf, int varOrFixed);
void readDataFromFile(char *buf);

void storeDataInVar(void);

void main(void)
{
	setupUART();

	_EINT();

	initUartBuffer();

	turnONSdCard();
	task = 6;

	while(1){

		cardStatus();

		switch (task) {
		case 0:
			sendEscapeCmd();
			task = 1;			// Avoid keep getting in this case statement by usiny a number different from any task
			break;

		case 1:
			createNewFile("new data.txt");
			task = 2;
			break;

		case 2:
			writeDataToFile("append data.txt", dataTest, 0);
			task = 3;
			break;

		case 3:
			readDataFromFile("read data.txt");
			task = 4;
			break;

		case 4:
			createNewFile("new data1.txt");
			//writeDataToFile("append data1.txt", uartBuffer, 1); //dataIn
			task = 5;
			break;

		case 5:
			readDataFromFile("read data.txt");
			while(storeDataFlag) {  } //wait until reading the UART finishes
			writeDataToFile("append data1.txt", uartBuffer, 1); //dataIn
			task = 9;
			break;

		case 6: //Sleep() was added to give time to the SD card/MSP430 read/write correctly
			createNewFile("new data.txt");
			sleep();
			writeDataToFile("append data.txt", dataTest, 0);
			sleep();
			createNewFile("new data1.txt");
			sleep();
			readDataFromFile("read data.txt");
			while(storeDataFlag) {blinkLED1n2(1);  } // wait until reading the UART finishes
			writeDataToFile("append data1.txt", uartBuffer, 1);
			task = 9;
			sleep();
			turnOFFSdCard();
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
	switch (byteRX) {
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
	blinkLED1(1);
	blinkLED2(1);
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
	UART_write(buf);
}

void writeDataToFile(const char *buf, char *data, int varOrFixed) {  // Assumes target file already exists

	UART_write(buf);      				// append cmd
	writeData(data, varOrFixed);		// write actual data
	sleep();
	cmdFlag = 0;
	sendEscapeCmd();					// escape: go back to cmd prompt '>'
}

void readDataFromFile(char *buf) {  // Assumes file to be read already exists
	UART_write(buf);
	storeDataFlag = 1;
}

void storeDataInVar(void) {
	int i = 0;

	for (i = 0; i < dataLength; ++i) {  // Store only those values in dataIn
		dataIn[i] = uartBuffer[i];
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

void writeData(char *buf, int varOrFixed) {
	int i; //, len;

	if (varOrFixed == 1) {			// Length of data variable, i.e. RXed from UART
		len = tempBuffCounter;
	}
	else if (varOrFixed == 0) { 	// Length of data fixed, i.e. from own sensors
		len = strlen(buf);
	}

	UART_putChar(START_BYTE);		// StartofLine marker

	for (i = 0; i < len; i++) {
		UART_putChar(buf[i]);
	}

	UART_putChar(END_BYTE);			// EndOfLine marker
	UART_putChar(CR);
}

void sleep(void)
{
	_delay_cycles(1000000);
}

void blinkLED1(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2;  // max is multiplied by two since toggling only gives half the On/Off cycle

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

void initUartBuffer(void) {
	int i = 0;
	for (i = 0; i < MAX_Temp_Buffer; i++) {
		uartBuffer[i] = '$';
	}
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     	// Store RXed character

	if (storeDataFlag == 1) {

		if (tempBuffCounter == MAX_Temp_Buffer-1) {
			tempBuffCounter = 0;
		}

		if (!pcktStartFlag && byteRX == START_BYTE) {
			pcktStartFlag = 1;
			tempBuffCounter = 0;
		}
		else if (pcktStartFlag && byteRX != END_BYTE) {
			uartBuffer[tempBuffCounter] = byteRX;
			tempBuffCounter++;
		}
		else if (pcktStartFlag && byteRX == END_BYTE) {
			dataLength = tempBuffCounter;
			pcktStartFlag = 0;
			lineCnter++;
		}

		else if (!pcktStartFlag && byteRX == '>') {
			storeDataInVar();
			storeDataFlag = 0;
		}
	}
}

