#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR    TIMERA1_VECTOR
#define TIMER0_A0_VECTOR    TIMERA0_VECTOR
#endif

volatile long tempRaw;

void FaultRoutine(void);
void ConfigWDT(void);
void ConfigClocks(void);
void ConfigPins(void);
void ConfigADC10(void);
void ConfigTimerA2(void);

void main(void)
{
  ConfigWDT();
  ConfigClocks();
  ConfigPins();
  ConfigADC10();
  ConfigTimerA2();

  while(1)
  {
   _bis_SR_register(LPM3_bits + GIE);
  }
}

void ConfigWDT(void)
 {
 WDTCTL = WDTPW + WDTHOLD;             	// Stop watchdog timer
 }

void ConfigClocks(void)
 {
 if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
   FaultRoutine();		         		// If calibration data is erased
 				               	   	   	// run FaultRoutine()
  BCSCTL1 = CALBC1_1MHZ; 		   		// Set range
  DCOCTL = CALDCO_1MHZ;  		   		// Set DCO step + modulation
  BCSCTL3 |= LFXT1S_2;                 	// LFXT1 = VLO
  IFG1 &= ~OFIFG;                      	// Clear OSCFault flag
  BCSCTL2 |= SELM_0 + DIVM_3 + DIVS_3; 	// MCLK = DCO/8, SMCLK = DCO/8
 }

void FaultRoutine(void)
 {
   P1OUT = BIT0;                       	// P1.0 on (red LED)
   while(1); 			         		// TRAP
 }

void ConfigPins(void)
 {
  P1DIR = ~BIT3;          				// P1.3 input, others output
  P1OUT = 0;              				// clear output pins
  P2SEL = ~(BIT6 + BIT7); 				// P2.6 and 7 GPIO
  P2DIR |= BIT6 + BIT7;	  				// P2.6 and 7 outputs
  P2OUT = 0;		  					// clear output pins
 }

void ConfigADC10(void)
 {
  ADC10CTL1 = INCH_10 + ADC10DIV_0;    	// Temp Sensor ADC10CLK
 }

void ConfigTimerA2(void)
 {
   CCTL0 = CCIE;
   CCR0 = 36000;
   TACTL = TASSEL_1 + MC_2;
 }

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
 {
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE ;
  _delay_cycles(4);                    	// Wait for ADC Ref to settle
  ADC10CTL0 |= ENC + ADC10SC;          	// Sampling and conversion start
  CCR0 +=36000;				   			// add 12 seconds to the timer
  _bic_SR_register_on_exit(LPM3_bits);
 }

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10 (void)
 {
  ADC10CTL0 &= ~ADC10IFG; 				// clear interrupt flag
  ADC10CTL0 &= ~ENC;			   		// Disable ADC conversion
  ADC10CTL0 &= ~(REFON + ADC10ON);     	// Ref and ADC10 off
  tempRaw = ADC10MEM;			   		// Read conversion value
  _bic_SR_register_on_exit(LPM3_bits);
 }
