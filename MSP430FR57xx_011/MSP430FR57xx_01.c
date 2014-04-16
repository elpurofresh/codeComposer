//******************************************************************************
//  MSP430FR57x Demo - Toggle P1.0 using software
//
//  Description: Toggle P1.0 using software.
//  ACLK = n/a, MCLK = SMCLK = TACLK = default DCO = ~625 KHz
//
//           MSP430FR5739
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |           P1.0|-->LED
//
//   Priya Thanigai
//   Texas Instruments Inc.
//   August 2010
//   Built with IAR Embedded Workbench Version: 5.10 & Code Composer Studio V4.0
//******************************************************************************

#include "msp430fr5739.h"


void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	P1DIR |= (BIT0 + BIT1 + BIT2);

	P1OUT |= BIT0;
	_delay_cycles(800000);
	P1OUT &= ~BIT0;

	P1OUT |= BIT1;
	_delay_cycles(800000);
	P1OUT &= ~BIT1;

	P1OUT |= BIT2;
	_delay_cycles(800000);
	P1OUT &= ~BIT2;

	while(1)
	{
		P2DIR |= BIT0;                            // LED interrupt
		P2OUT &= ~BIT0;
		//P2OUT ^= BIT0;
		//__delay_cycles(100000);
	}
}
