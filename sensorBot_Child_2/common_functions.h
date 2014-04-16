/*
 * common_functions.h
 *
 *  Created on: Jul 17, 2012
 *      Author: andres
 */

#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_

#include <string.h>
#include <stdlib.h>
#include "common_variables.h"

void setupUART(void);
void configClocks(void);
void setupTimer(void);
void setupOutputs(void);
void turnTXD(int enable);
void turnRXD(int enable);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);
void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

void UART_putChar (char c);
void UART_putInt  (int x);

int checkSDCardStatus();						//checks to make sure all slaves are working properly
void turnONSdCard(void);
void turnOFFSdCard(void);

void createNewFile(const char *buf) ;
void UART_write (const char *buf);
void writeData(char *buf, int varOrFixed);
void writeDataToFile(const char *buf, char *data, int varOrFixed);
void sendEscapeCmd(void);

void itoa(int n, char s[]);
void reverse(char *string);
int string_length(char *pointer);
int string_length(char *pointer);

void getData(void);

void configClocks(void)
{
	WDTCTL = WDTPW + WDTHOLD;		// Stop WDT
	BCSCTL1 = CALBC1_1MHZ;          // Set range for calibrated 1.1 MHz (Basic Timer = DCO)
	DCOCTL = CALDCO_1MHZ;			// Set DCO step and modulation 1MHz

	//BCSCTL2 = LFXT1S_0;				// Set the 2nd Basic Timer module to the ACLK => Xtal @ 32.768kHz

	/*UCA0CTL1 |= UCSSEL_2;			// SMCLK
	UCA0BR0 = 104;					// 1MHz 9600
	UCA0BR1 = 0;			        // 1MHz 9600
	UCA0MCTL = UCBRS0;				// Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;			// **Initialize USCI state machine**

	 */}

void setupTimer(void){				//enable clock interrupt

	CCTL0 = CCIE;
	CCR0 = 32768-1;
	TACTL = TASSEL_1 + MC_1 + TACLR;		// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, clear
}

void setupUART(void)
{
	// Setup the baudrate and modulation for the USCI_A (UART module)
	UCA0CTL1 |= UCSSEL_2;			// SMCLK
	UCA0BR0 = 104;					// 1MHz 9600
	UCA0BR1 = 0;			        // 1MHz 9600
	UCA0MCTL = UCBRS0;				// Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;			// **Initialize USCI state machine**

	P1DIR = 0xFF & ~UART_RXD;      	// All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                      // All P1.x reset
	P1SEL = UART_RXD + UART_TXD;    // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD + UART_TXD; 	// P1.1 = RXD, P1.2=TXD
	IE2 |= UCA0RXIE;                // Enable USCI_A0 RX interrupt
}

void setupOutputs(void)
{
	P1DIR |= RED_LED + GREEN_LED;
	P1OUT &= ~(RED_LED + GREEN_LED);

	P2DIR |= TX_Enable + RX_Enable;		// Set output controls on TX/RX switch (Inter robot comm)
	P2OUT &= ~(TX_Enable + RX_Enable);	// Turn both OFF  (Inter robot comm)
}

void turnTXD(int enable)
{
	if (enable == ON) {
		P2OUT |= TX_Enable;
	}
	else {
		P2OUT &= ~TX_Enable;
	}
}

void turnRXD(int enable)
{
	if (enable == ON) {
		P2OUT |= RX_Enable;
	}
	else {
		P2OUT &= ~RX_Enable;
	}
}

void blinkLEDRed(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= RED_LED;             	// Toggle P1.0 using exclusive-OR
		do j--;							// Make some delay
		while (j != 0);
	}
}

void blinkLEDGreen(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= GREEN_LED;              // Toggle P1.0 using exclusive-OR
		//do j--;						// Make some delay
		//while (j != 0);
		sleepHalf();
	}
}

void blinkLEDRednGreen(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= RED_LED;              // Toggle P1.0 using exclusive-OR
		P1OUT ^= GREEN_LED;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
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

void UART_putChar (char charToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data
	//byteRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

	uart_Out[cnterTest1] = UCA0TXBUF;
	cnterTest1++;

	if (cnterTest1 > 60) {
		cnterTest1 = 0;
	}
}

void UART_putInt (int IntToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = IntToSend;       // send data
	//byteRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}

void turnONSdCard(void) {
	//P1DIR |= SD_Enable0 + SD_Enable1 + SD_Enable2;
	P1DIR |= SD_Enable1 + SD_Enable2;
	//P1DIR |= SD_Enable2;
	//P1OUT |= SD_Enable0 + SD_Enable1 + SD_Enable2;
	P1OUT |= SD_Enable1 + SD_Enable2;
	//P1OUT &= ~SD_Enable2;
	sleep();
}

void turnOFFSdCard(void)
{
	//P1OUT &= ~(SD_Enable0 + SD_Enable1 + SD_Enable2);
	P1OUT &= ~(SD_Enable1 + SD_Enable2);
	//P1OUT |= SD_Enable2;
}

void createNewFile(const char *buf) {
	UART_write(buf);
}

void UART_write (const char *buf) {
	int i;
	for (i = 0; i < strlen(buf); i++) {
		UART_putChar (buf[i]);
	}

	UART_putChar(CR);
}

void writeData(char *buf, int varOrFixed) {
	int i, len;

	if (varOrFixed == 0) {				// Length of data fixed, i.e. from own sensors
		len = strlen(buf);
	}
	else if (varOrFixed == 1) {			// Length of data variable, i.e. RXed from UART
		len = dataLength;
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

void writeDataToFile(const char *buf, char *data, int varOrFixed) {  // Assumes target file already exists

	UART_write(buf);      				// append cmd
	writeData(data, varOrFixed);		// write actual data
	sleep();
	sdCardCmdFlag = 0;
	sendEscapeCmd();					// escape: go back to cmd prompt '>'
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

int checkSDCardStatus() {
	char sdCardOK = 0;
	int i = 0, timeOut = 10;

	turnRXD(OFF); 			// Turn OFF the Inter robot communication

	turnONSdCard();
	while(i < timeOut) {

		UART_putChar(CR);
		createNewFile("new myData.txt");
		sleep();
		writeDataToFile("append myData.txt", header, 0);
		sleep();

		if (byteRX == '>') {
			sleep();			//we need to figure out why this is necessary
			sdCardOK = 1;
			blinkLEDRednGreen(1);
			i = timeOut;
		} else {
			blinkLEDRed(1);		//indicates that the condition failed to satisfy the if statement
		}

		i++;
	}
	turnOFFSdCard();

	turnRXD(ON);		// Turn ON the Inter robot communication

	if (sdCardOK) {		//checks only the desired slave is working
		return 1;
	} else {
		return 0;		//desired slave is not functioning properly
	}
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
	int i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void reverse(char *string)
{
	int length, c;
	char *begin, *end, temp;

	length = string_length(string);  //may want to change this to strnlen!!

	begin = string;
	end = string;

	for ( c = 0 ; c < ( length - 1 ) ; c++ )
		end++;

	for ( c = 0 ; c < length/2 ; c++ )
	{
		temp = *end;
		*end = *begin;
		*begin = temp;

		begin++;
		end--;
	}
}

int string_length(char *pointer)
{
	int c = 0;

	while( *(pointer+c) != '\0' )
		c++;

	return c;
}

void getData(void)
{
	OX = 65/*rand() % 100*/;
	AB = 43/*rand() % 100*/;
	TP = 21/*rand() % 100*/;
	PH = 01/*rand() % 15*/;
}


#endif /* COMMON_FUNCTIONS_H_ */
