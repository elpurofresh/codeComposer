#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void setupUART(void);
void configClock(void);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);

void turnONSdCard(void);
void turnOFFSdCard(void);
void cardStatus(void);
void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

void initUartBuffer(void);

void UART_putChar (char c);
void UART_write (const char *buf);

void sendEscapeCmd(void);
void createNewFile(const char *buf);
void writeDataToFile(const char *buf, char *data, int varOrFixed);
void writeData(char *buf, int varOrFixed);
void readDataFromFile(char *buf);

void storeDataInVar(void);

char checkStatus(int);						//checks to make sure all slaves are working properly

int getRandomBit(void);

void setupUART(void)
{
	P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                // All P1.x reset
	P1SEL = UART_RXD + UART_TXD;              // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD + UART_TXD;             // P1.1 = RXD, P1.2=TXD
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void configClock(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void cardStatus(void)
{
	switch (byteRX) {
	case '<':
		sdCardCmdFlag	= 0;
		break;
	case '>':
		sdCardCmdFlag	= 1;
		break;
	}

	sleep();
}

void turnONSdCard(void) {
	P1DIR |= SD_Enable0 + SD_Enable1 +SD_Enable2;
	P1OUT |= SD_Enable0 + SD_Enable1 +SD_Enable2;
}

void turnOFFSdCard(void) {
	P1OUT &= ~(SD_Enable0 + SD_Enable1 +SD_Enable2);
}

void sendEscapeCmd(void) {

	if (sdCardCmdFlag == 0) {
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
	sdCardCmdFlag = 0;
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
	int i;

	if (varOrFixed == 0) {				// Length of data fixed, i.e. from own sensors
		len = strlen(buf);
	}
	else if (varOrFixed == 1) {			// Length of data variable, i.e. RXed from UART
		len = tempBuffCounter;
	}
	else if (varOrFixed == 2) {			// Length of known data, i.e. DAC value/CLK value
		len = 15;						// in this case, length of data is two bytes.
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

void sleepHalf(void)
{
	_delay_cycles(500000);
}

void sleepQuart(void)
{
	_delay_cycles(250000);
}

void blinkLEDRed(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2;  // max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; i++) {
		P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
}

void blinkLEDGreen(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= BIT6;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
}

void blinkLEDRednGreen(int numOfBlinks)
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

char checkStatus(int address)
{
	char sdCardOK = 0, clkOK = 0;
	int i = 0, timeOut = 10;

	switch(address){

	//check SD Card

	case SD_CHECK:
		P2OUT |= BIT2;
		turnONSdCard();
		while(i < timeOut) {

			UART_putChar(CR);
			//UART_putChar('X');
			//cardStatus();

			if (byteRX == '>') {
				sleep();			//we need to figure out why this is necessary
				sdCardOK = 1;
				turnOFFSdCard();
				blinkLEDGreen(2);
				i = timeOut;
			}

			blinkLEDRed(1);			//indicates that the condition failed to satisfy the if statement
			i++;
		}
		turnOFFSdCard();
		P2OUT &= ~BIT2;
		break;


		//check clock

	case CLK_CHECK:
		i = 0;

		P2OUT |= CLK_Enable;

		while(i < timeOut){
			UART_putChar('C');
			UART_putChar(CR);

			if (byteRX == 'K'){
				clkOK = 1;
				blinkLEDGreen(2);
				i = timeOut;
			}

			blinkLEDRed(1);
			i++;


		}

		P2OUT &= ~CLK_Enable;
		break;


	}
	if (sdCardOK || clkOK) {		//checks only the desired slave is working
		return 1;
	} else {
		return 0;							//desired slave is not functioning properly
	}
}

#endif /*COMMON_FUNCTIONS_H_*/
