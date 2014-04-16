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

volatile unsigned int j;            // volatile to prevent optimization

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;           	// Stop watchdog timer
	P1DIR |= BIT1 + BIT2;               	// Set P1.0 to output direction
	P1OUT &= ~BIT1;							// Set P1.0 to LOW, LED off
	P1OUT &= ~BIT2;							// Set P1.6 to LOW, LED off

	j = 10000;                          // SW Delay

	while(1)
	{
		P1OUT ^= BIT1;                      // Toggle P1.0 using exclusive-OR
		P1OUT ^= BIT2;

		do j--;
		while (j != 0);
	}
}

