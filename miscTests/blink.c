//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  J. Stevenson
//  Texas Instruments, Inc
//  July 2011
//  Built with Code Composer Studio v5
//***************************************************************************************

#include <msp430g2553.h>

volatile unsigned int i;            // volatile to prevent optimization

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;           	// Stop watchdog timer
	P1DIR |= BIT0 + BIT6;               	// Set P1.0 to output direction
	P1OUT &= ~BIT0;							// Set P1.0 to LOW, LED off
	P1OUT &= ~BIT6;							// Set P1.6 to LOW, LED off

	P1REN |= BIT3;							// P1.3 Enable Pullup/Pulldown
	P1OUT = BIT3;							// P1.3 Pullup
	P1IES |= BIT3; 							// P1.3 Hi/Lo edge
	P1IE |= BIT3;							// P1.3 Interrupt enabled
	P1IFG &= ~BIT3;							// P1.3 IFG Cleared just in case

	_EINT();

	i = 10000;                          // SW Delay

	while(1)
	{
		if (P1IFG & BIT3)					// P1.3 IFG Clearead
		{
			P1IFG &= ~BIT3;					// P1.3 IFG Clearead
			P1OUT ^= BIT0;					// Toggle LED at P1.0
		}


		P1OUT ^= BIT6;                      // Toggle P1.0 using exclusive-OR


		do i--;
		while (i != 0);
	}
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1OUT ^= BIT0;					// Toggle LED at P1.0
	P1IFG &= ~BIT3;					// P1.3 IFG Clearead
}
