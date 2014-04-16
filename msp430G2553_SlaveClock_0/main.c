#include "msp430g2553.h"
#include "common_variables.h"
#include "common_functions.h"

#define LED1 	BIT0
#define LED2	BIT6

volatile int flag = 0;

void main(void){

	configClock();
	setupPorts();

	setupTimer();

	_EINT();   // Not really necessary since CPU is going to LPM3 @ the main

	TACCTL0 &= ~CCIE;

	while(1)
	{
		__low_power_mode_3(); //enter LPM3 w/GIE
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	P1OUT |= LED1;
	P1OUT |= BIT4;

	P1OUT &= BIT4;
	P1OUT &= LED1;

	__low_power_mode_off_on_exit();    // Restores active mode on return
}


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1OUT |= LED2;
	__delay_cycles(100);

	//TACCTL0 ^= CCIE;			// Toggle the timer interrupt enable

	if (flag == 1) {
		TACCTL0 &= ~CCIE;
		TA0CCR0 = 0;

		flag = 0;
	} else {
		TA0CCR0 = 32768-1;
		TACCTL0 |= CCIE;

		flag = 1;
	}

	P1IES ^= BIT5;				// Change the edge detection from Lo/Hi <-> Hi/Lo
	P1IFG &= ~BIT5;         	// Clear P1.5 IFG

	P1OUT &= ~LED2;

	__low_power_mode_off_on_exit();    // Restores active mode on return
}

