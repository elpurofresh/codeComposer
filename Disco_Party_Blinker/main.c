#include "msp430fr5739.h"

int delay_function(void);
/*
 * main.c
 */

volatile unsigned int i = 0;
volatile unsigned int delay;

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;

	P2DIR |= BIT0;

	for(;;){

		P2OUT |= BIT0;
		delay_function();
		P2OUT = 0;
		delay_function();
		delay++;
	}
}


int delay_function(){

	if(delay >= 1 && delay < 4){
		_delay_cycles(1000000);
	}

	if(delay >= 4 && delay < 7){
		_delay_cycles(500000);
	}

	if(delay >= 7 && delay < 10){
		_delay_cycles(250000);
	}

	if(delay >= 10 && delay < 13){
		_delay_cycles(125000);
	}

	if(delay >= 13 && delay < 19){
		_delay_cycles(62500);
	}

	if(delay >= 19 && delay < 22){
		_delay_cycles(1000000);
	}

	if(delay >= 22){
		P2OUT = 0;
		_delay_cycles(2000000);
		delay=0;
	}
}
