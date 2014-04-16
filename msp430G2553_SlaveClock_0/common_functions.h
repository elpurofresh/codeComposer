#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void setupUART(void);
void configClock(void);
void setupPorts(void);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);

void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

void UART_putChar (char c);
void UART_write (const char *buf);

void initUartBuffer(void);
void timeKeeper(void);


void setupUART(void)
{
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

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
}

void setupPorts(void){
	P1DIR |= 0xFF;							  // all P1.x outputs
	P1OUT &= 0;								  // all P1.x reset

	P2DIR |= 0xFF;							  // all P1.x outputs
	P2OUT = 0;								  // all P1.x reset

	P1DIR &= ~BIT5;		// P1.5 set to input
	//P1REN |= BIT5;		// P1.5 pull-up set on it
	P1OUT &= ~BIT5;		// P1.5	set to pull-up (input port)
	//P1IN  |= BIT5;		// P1.5 set to Low
	P1IES &= ~BIT5;		// P1.5 Lo/Hi edge interrupt
	P1IE  |= BIT5;		// P1.5 interrupt enabled
	P1IFG &= ~BIT5;		// P1.5 IFG cleared
}


void setupTimer(void)
{
	TA0CCR0 = 32768-1;							// Count from 0-32767
	TA0CCTL0 |= CCIE;							// Enable clock interrupt
	TA0CTL = TASSEL_1 + MC_1 + TACLR;			// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, clear
}

/*
 * Put one character in the UART bus
 */
void UART_putChar (char charToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data
}

/**
 * Write string buffer to UART
 */
void UART_write (const char *buf)
{
	int i;
	for (i = 0; i < strlen(buf); i++) {
		UART_putChar (buf[i]);
	}
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
		uartTime[i] = '$';
	}
}

void timeKeeper(void)
{
	P1OUT ^= BIT0;

	if (sec == 59) { sec = 0; min ++; }

	if ( (sec % 15) == 0 ) {
		P1OUT |= BIT4;
		__delay_cycles(100000);
		P1OUT &= ~BIT4;
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }
	P1OUT ^= BIT0;
}



#endif /*COMMON_FUNCTIONS_H_*/
