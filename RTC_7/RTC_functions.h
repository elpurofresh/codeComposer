/*
 * RTC_functions.h
 *
 *  Created on: Jun 5, 2012
 *      Author: Greg Wells
 */

#ifndef RTC_FUNCTIONS_H_
#define RTC_FUNCTIONS_H_

void setupUART(void);
void configClocks(void);
void setupTimer(void);
void setupOutputs(void);
void turnTXD(int enable);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);
void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

void UART_putChar (char c);
void UART_putInt  (int x);

void setupUART(void)
{
	P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                // All P1.x reset
	P1SEL = UART_RXD;               		  // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD; 				          // P1.1 = RXD, P1.2=TXD
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void turnTXD(int enable)
{

	if (enable == ON) {

		P1SEL = UART_RXD + UART_TXD;		     // P1.1 = RXD, P1.2=TXD		Refer to MSP430x2xx Family User's Guide for P1SEL & P1SEL2 explanation
		P1SEL2 = UART_RXD + UART_TXD;             // P1.1 = RXD, P1.2=TXD
	}

	else {
		P1SEL &= ~(UART_TXD);		     // P1.1 = RXD, P1.2=TXD		Refer to MSP430x2xx Family User's Guide for P1SEL & P1SEL2 explanation
		P1SEL2 &= ~(UART_TXD);             // P1.1 = RXD, P1.2=TXD
		P1OUT &= ~UART_TXD;                                // All P1.x reset
	}
}

void configClocks(void)
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

void setupTimer(void){						//enable clock interrupt

	CCTL0 = CCIE;
	CCR0 = 32768;
	TACTL = TASSEL_1 + MC_1;
}

void setupOutputs(void)
{
	P1DIR |= Red_LED + Green_LED + MasterNote;
	P1OUT &= ~(Red_LED + Green_LED + MasterNote);
}

void blinkLEDRed(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= Red_LED;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
}

void blinkLEDGreen(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= Green_LED;              // Toggle P1.0 using exclusive-OR
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

}

void UART_putInt (int IntToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = IntToSend;       // send data
	//byteRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}

#endif /* RTC_FUNCTIONS_H_ */
