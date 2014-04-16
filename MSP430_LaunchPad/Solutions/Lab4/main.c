#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR    TIMERA1_VECTOR
#define TIMER0_A0_VECTOR    TIMERA0_VECTOR
#endif

volatile long tempRaw;

void FaultRoutine(void);

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog timer

  P1DIR = 0x41;                        // P1.0 & 6 output (red/green LEDs)
  P1OUT = 0;                           // LEDs off

if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
   FaultRoutine();		               // If calibration data is erased
 				                       // run FaultRoutine()
  BCSCTL1 = CALBC1_1MHZ; 		       // Set range
  DCOCTL = CALDCO_1MHZ;  		       // Set DCO step + modulation

  BCSCTL3 |= LFXT1S_2;                 // LFXT1 = VLO
  IFG1 &= ~OFIFG;                      // Clear OSCFault flag
  BCSCTL2 |= SELM_0 + DIVM_3 + DIVS_3; // MCLK = DCO/8

  while(1)
  {
	ADC10CTL1 = INCH_10 + ADC10DIV_0;  // Temp Sensor ADC10CLK
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
	_delay_cycles(5);                  // Wait for ADC Ref to settle
	ADC10CTL0 |= ENC + ADC10SC;        // Sampling and conversion start

    P1OUT = 0x40; 			    	   // green LED on
    _delay_cycles(100);

    ADC10CTL0 &= ~ENC;
    ADC10CTL0 &= ~(REFON + ADC10ON);
    tempRaw = ADC10MEM;


    P1OUT = 0; 		                   // green LED off
    _delay_cycles(125000);
  }
}

void FaultRoutine(void)
 {
   P1OUT = 0x01;                       // red LED on
   while(1); 			               // TRAP
 }
