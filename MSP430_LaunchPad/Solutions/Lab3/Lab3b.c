#include  "msp430g2553.h"

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR    TIMERA1_VECTOR
#define TIMER0_A0_VECTOR    TIMERA0_VECTOR
#endif

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;		// watchdog timer setup

	P1DIR = 0x41;					// I/O setup
	P1OUT = 0x01;

	BCSCTL3 |= LFXT1S_0;			// clock system setup

	while(IFG1 & OFIFG)				// wait for OSCFault to clear
	{
	  IFG1 &= ~OFIFG;
	  _delay_cycles(100000);
	}

	P1OUT = 0;						// both LEDs off

	_bis_SR_register(SCG1 + SCG0);	// clock system setup
	BCSCTL2 |= SELM_3 + DIVM_3;
	
	while(1)
	{
	  P1OUT = 0x40; 			    // LED on
	  _delay_cycles(100);
	  P1OUT = 0; 			    	// LED off
	  _delay_cycles(5000);
	}
}
