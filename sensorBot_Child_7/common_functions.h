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


void initClocks(void);						//initializes MCLK and ACLK
void initUARTBot(void);						//initializes UART0
void initUARTSdCard(void);					//initializes UART1
void initUartBuffer(void);

void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

void initLEDs(void);
void blinkLED1(int numOfBlinks);
void blinkLED2(int numOfBlinks);
void blinkLED3(int numOfBlinks);
void blinkLED4(int numOfBlinks);
void blinkLED5(int numOfBlinks);
void blinkLED6(int numOfBlinks);
void blinkLED7(int numOfBlinks);
void blinkLED8(int numOfBlinks);
void initLEDWait(void);

void UART_SDputChar(char charToSend);
void UART_BotputChar(char charToSend);

int checkSDCardStatus();						//checks to make sure all slaves are working properly
void turnSdCard(int val);

void createNewFile(const char *buf) ;
void UART_SDwrite (const char *buf);
void writeDataSD(char *buf, int varOrFixed);
void writeDataToSDFile(const char *buf, char *data, int varOrFixed);
void sendEscapeCmd(void);

void getData(void);

void StartUpSequence(void);
void LongDelay();

void initClocks(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	// XT1 Setup
	PJSEL0 |= BIT4 + BIT5;

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELA_0 + SELS_3 + SELM_3;        // set ACLK = XT1; MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
	CSCTL4 |= XT1DRIVE_0;
	CSCTL4 &= ~XT1OFF;
	// CSCTL0_H = 0x01;                          // Lock Register

	do
	{
		CSCTL5 &= ~XT1OFFG;
		// Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
}

void initUARTBot(void)
{
	// Configure UART 0
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK = 8MHz as UCBRCLK
	UCA0BR0 = 52;                           	// (8000000Hz / 9600bps) / 16  = 52.083
	UCA0BR1 = 0;
	// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	// 52.083-52 = 0.083 *16 = 1.33
	// UCBRSx value 0.3335= 0x42 (See UG)
	UCA0MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)
	UCA0CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= UART_RXBot + UART_TXBot;
	P2SEL0 &= ~(UART_RXBot + UART_TXBot);

	UCA0IE |= UCRXIE;                         	// Enable RX interrupt
}

void initUARTSdCard(void)
{
	// Configure UART 1
	UCA1CTL1 |= UCSWRST;
	UCA1CTL1 = UCSSEL_2;                    	// Set SMCLK = 8MHz as UCBRCLK
	UCA1BR0 = 52;                 				// (8000000Hz / 9600bps) / 16  = 52.083
	UCA1BR1 = 0;
	// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	// 52.083-52 = 0.083 *16 = 1.33
	// UCBRSx value 0.3335= 0x42 (See UG)
	UCA1MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)
	UCA1CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= UART_RXSd + UART_TXSd;
	P2SEL0 &= ~(UART_RXSd + UART_TXSd);

	UCA1IE |= UCRXIE;                         	// Enable RX interrupt
}

void setupTimer(void)
{
	TA0CCR0 = 32768-1;							// Count from 0-32767
	TA0CCTL0 |= CCIE;							// Enable clock interrupt
	TA0CTL = TASSEL_1 + MC_1 + TACLR;			// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, clear
}

void initUartBuffer(void)
{
	int i = 0;
	for (i = 0; i < strlen(uartBuffer); ++i) {
		uartBuffer[i] = '@';
	}
}

void sleep(void){
	_delay_cycles(8000000);
}

void sleepHalf(void)
{
	_delay_cycles(4000000);
}

void sleepQuart(void)
{
	_delay_cycles(2000000);
}

void initLEDs(void)
{
	// Enable LEDs
	P3DIR |= BIT4+BIT5+BIT6+BIT7;
	P3OUT &= ~(BIT4+BIT5+BIT6+BIT7);
	PJDIR |= BIT0 +BIT1+BIT2+BIT3;
	PJOUT &= ~(BIT0+BIT1+BIT2+BIT3);
}

void UART_SDputChar(char charToSend)  	// UART1 Transmit Subroutine
{
	while (!(UCTXIFG & UCA1IFG));  // USCI_A1 TX buffer ready?
	UCA1TXBUF = charToSend;       // send data

	uart_Out[cnterTest1] = UCA1TXBUF;
	cnterTest1++;

	if (cnterTest1 > 60) {
		cnterTest1 = 0;
	}
}

void UART_BotputChar(char charToSend)  	// UART0 Transmit Subroutine
{
	while (!(UCTXIFG & UCA0IFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data

	uart_Out[cnterTest1] = UCA0TXBUF;
	cnterTest1++;

	if (cnterTest1 > 60) {
		cnterTest1 = 0;
	}
}

void turnSdCard(int val) {

	if (val == ON) {
		P2DIR |= SD_Enable0;// + SD_Enable1;
		P2OUT |= SD_Enable0;// + SD_Enable1;
		sleep();
	}
	else if (val == OFF) {
		P2OUT &= ~SD_Enable0;
	}
}

void createNewFile(const char *buf) {
	UART_SDwrite(buf);
}

void UART_SDwrite (const char *buf) {
	int i;
	for (i = 0; i < strlen(buf); i++) {
		UART_SDputChar (buf[i]);
	}

	UART_SDputChar(CR);
}

void writeDataSD(char *buf, int varOrFixed) {
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

	UART_SDputChar(START_BYTE);		// StartofLine marker

	for (i = 0; i < len; i++) {
		UART_SDputChar(buf[i]);
	}

	UART_SDputChar(END_BYTE);			// EndOfLine marker
	UART_SDputChar(CR);
}

void writeDataToSDFile(const char *buf, char *data, int varOrFixed) {  // Assumes target file already exists

	UART_SDwrite(buf);      				// append cmd
	writeDataSD(data, varOrFixed);		// write actual data
	sleep();
	//sdCardCmdFlag = 0;
	sendEscapeCmd();					// escape: go back to cmd prompt '>'
}

void sendEscapeCmd(void) {

	//if (sdCardCmdFlag == 0) {
		UART_SDputChar(CTRLZ);
		UART_SDputChar(CTRLZ);
		UART_SDputChar(CTRLZ);
		sleepQuart(); // wait for a 1/4sec

	/*} else {
		UART_SDputChar(CR);
	}*/
}

int checkSDCardStatus() {
	char sdCardOK = 0;
	int i = 0, timeOut = 10;

	turnSdCard(ON);
	while(i < timeOut) {

		blinkLED1(1);

		UART_SDputChar(CR);
		createNewFile("new myData.txt");
		sleepQuart();
		//sleep();
		writeDataToSDFile("append myData.txt", header, 0);
		sleepQuart();
		//sleep();

		if (byteRX_SD == '>') {
			sleep();			// we need to figure out why this is necessary
			sdCardOK = 1;
			blinkLED2(1);		// display success
			i = timeOut;
		} else {
			blinkLED8(1);		// indicates that the condition failed to satisfy the if statement
		}

		i++;
	}
	turnSdCard(OFF);

	if (sdCardOK) {		// SD card passed/responded correctly
		return 1;
	} else {
		return 0;		// SD card did not pass/respond correctly
	}
}

void blinkLED1(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		PJOUT ^= BIT0;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED2(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		PJOUT ^= BIT1;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED3(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		PJOUT ^= BIT2;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED4(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		PJOUT ^= BIT3;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED5(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P3OUT ^= BIT4;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED6(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P3OUT ^= BIT5;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED7(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P3OUT ^= BIT6;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void blinkLED8(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P3OUT ^= BIT7;             	// Toggle P1.0 using exclusive-OR
		sleepHalf();
	}
}

void initLEDWait(void)
{
	PJOUT |= BIT0;
	__delay_cycles(5000000);
	PJOUT |= BIT1;
	__delay_cycles(5000000);
	PJOUT |= BIT2;
	__delay_cycles(5000000);
	PJOUT |= BIT3;
	__delay_cycles(5000000);
	P3OUT |= BIT4;
	__delay_cycles(5000000);
	P3OUT |= BIT5;
	__delay_cycles(5000000);
	P3OUT |= BIT6;
	__delay_cycles(5000000);
	P3OUT |= BIT7;
	__delay_cycles(5000000);
	P3OUT &= ~(BIT4+BIT5+BIT6+BIT7);
	PJOUT &= ~(BIT0+BIT1+BIT2+BIT3);
}

void getData(void)
{
	OX = 65/*rand() % 100*/;
	AB = 43/*rand() % 100*/;
	TP = 21/*rand() % 100*/;
	PH = 01/*rand() % 15*/;
}

void StartUpSequence(void)  //this is copied from the Ti Fraunch Pad experience. Only thing it does is make a fancy LED sequence on boot.
{
	unsigned char flag=4,up=1,counter = 0;
	unsigned char LED_ArrayPJ[] = {0x01,0x02,0x04,0x08};
	unsigned char LED_ArrayP3[] = {0x80,0x40,0x20,0x10};

	while (counter <10)
	{
		counter++;
		PJOUT &= ~(BIT0 +BIT1+BIT2+BIT3);
		P3OUT &= ~(BIT4 +BIT5+BIT6+BIT7);
		if(up)
		{
			while(flag)
			{
				P3OUT = LED_ArrayP3[flag-1];
				PJOUT = LED_ArrayPJ[flag-1];
				LongDelay();
				flag--;
			}
			up=0;
		}
		else
		{
			while(flag<4)
			{
				P3OUT = LED_ArrayP3[flag];
				PJOUT = LED_ArrayPJ[flag];
				LongDelay();
				flag++;
			}
			up = 1;
		}
	}
	PJOUT &= ~(BIT0 +BIT1+BIT2+BIT3);
	P3OUT &= ~(BIT4 +BIT5+BIT6+BIT7);
}

void LongDelay()
{
	__delay_cycles(250000);
}

/*void turnBotRX(int val)
{
	if (val == ON) {
		allowISR = 1;
	} else {
		allowISR = 0;
	}

}*/


#endif /* COMMON_FUNCTIONS_H_ */
