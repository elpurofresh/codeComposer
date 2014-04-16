#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void configClock(void);
void setupPorts(void);

void blinkLEDRed(int numOfBlinks);
void blinkLEDGreen(int numOfBlinks);
void blinkLEDRednGreen(int numOfBlinks);

void sleep(void);							//delays the processor for 1 sec
void sleepHalf(void);						//delays the processor for 1/2 sec
void sleepQuart(void);						//delays the processor for 1/4 sec

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



#endif /*COMMON_FUNCTIONS_H_*/
