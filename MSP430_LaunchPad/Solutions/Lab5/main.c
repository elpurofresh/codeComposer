#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR    TIMERA1_VECTOR
#define TIMER0_A0_VECTOR    TIMERA0_VECTOR
#endif

volatile long tempRaw;
volatile unsigned int i;

void FaultRoutine(void);
void ConfigWDT(void);
void ConfigClocks(void);
void ConfigLEDs(void);
void ConfigADC10(void);
void ConfigTimerA2(void);

void main(void)
{
  ConfigWDT();
  ConfigClocks();
  ConfigLEDs();
  ConfigADC10();
  ConfigTimerA2();

  _BIS_SR(GIE);

  while(1)
  {
   P1OUT |= BIT0;
   for (i = 100; i > 0; i--);
   P1OUT &= ~BIT0;
   for (i = 5000; i > 0; i--);
  }
}

void ConfigWDT(void)
 {
 WDTCTL = WDTPW + WDTHOLD;                 	// Stop watchdog timer
 }

void ConfigClocks(void)
 {
 if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
   FaultRoutine();		                    // If calibration data is erased
 				                            // run FaultRoutine()
  BCSCTL1 = CALBC1_1MHZ; 					// Set range
  DCOCTL = CALDCO_1MHZ;  					// Set DCO step + modulation
  BCSCTL3 |= LFXT1S_2;                      // LFXT1 = VLO
  IFG1 &= ~OFIFG;                           // Clear OSCFault flag
  BCSCTL2 |= SELM_0 + DIVM_3 + DIVS_3;      // MCLK = DCO/8, SMCLK = DCO/8
 }

void FaultRoutine(void)
 {
   P1OUT = BIT0;                            // P1.0 on (red LED)
   while(1); 			                    // TRAP
 }

void ConfigLEDs(void)
 {
  P1DIR = BIT6 + BIT0;                      // P1.6 and P1.0 outputs
  P1OUT = 0;                                // LEDs off
 }

void ConfigADC10(void)
 {
  ADC10CTL1 = INCH_10 + ADC10DIV_0;        // Temp Sensor ADC10CLK
 }

void ConfigTimerA2(void)
  {
   CCTL0 = CCIE;
   CCR0 = 12000;
   TACTL = TASSEL_1 + MC_2;
  }

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
  _delay_cycles(5);                         // Wait for ADC Ref to settle
  ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
  P1OUT |= BIT6; 			                // P1.6 on (green LED)
  _delay_cycles(100);
  ADC10CTL0 &= ~ENC;				   		// Disable ADC conversion
  ADC10CTL0 &= ~(REFON + ADC10ON);        	// Ref and ADC10 off
  tempRaw = ADC10MEM;						// Read conversion value
  P1OUT &= ~BIT6; 				                // green LED off
  CCR0 +=12000;								// add 12 seconds to the timer
}
