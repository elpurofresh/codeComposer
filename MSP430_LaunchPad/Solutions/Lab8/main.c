#include <msp430.h>
#include <ti/mcu/msp430/csl/CSL.h>

int main(void)
{
    CSL_init();                        // Activate Grace config
    
    while (1)
    {
     _bis_SR_register(LPM3_bits);		// Enter LPM3
     _delay_cycles(10000);				// 10ms delay
     P1OUT &= ~BIT0;					// Turn off LED on P1.0
    }
}

void timerIsr(void)
{
	P1OUT = BIT0;						// Turn on LED on P1.0
}
