#ifndef DAC_FUNCTIONS_H_
#define DAC_FUNCTIONS_H_

#include "DAC_variables.h"

void setupUART(void);
void configClocks(void);						//set DCO to 1MHz and baud rate to 96000
void setupInterrupts(void);
void setupOutputs(void);
void turnTXD(int enable);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);

void UART_putChar (char c);
int getRandomNumber(void);
void enableDAC(char port1, char port2);

void setupUART(void)
{
	P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                // All P1.x reset
	P1SEL = UART_RXD; // + UART_TXD;              // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD; // + UART_TXD;             // P1.1 = RXD, P1.2=TXD
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

void setupInterrupts(void)
{
	P1IN = ADR_BUS0;
	P1IE |= ADR_BUS0;
	P1IES |= ADR_BUS0;
	P1IFG &= ~ADR_BUS0;

}

void setupOutputs(void)
{
	P1DIR |= Red_LED + Green_LED;
	P1OUT &= ~(Red_LED + Green_LED);
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
	//byteRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}

/*int getRandomNumber(void)
{
	ADC10CTL1 |= INCH_5;
	ADC10CTL0 |= SREF_1 + ADC10SHT_1 + REFON + ADC10ON;
	ADC10CTL0 |= ENC + ADC10SC;
	while(ADC10CTL1 & ADC10BUSY);
	randNumber=ADC10MEM;
	return randNumber;
}*/




#endif /*DAC_FUNCTIONS_H_*/
