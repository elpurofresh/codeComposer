#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR    TIMERA1_VECTOR
#define TIMER0_A0_VECTOR    TIMERA0_VECTOR
#endif

#define TXD BIT1							// TXD on P1.1
#define RXD	BIT2     						// RXD on P1.2
#define Bitime 13*4							// 0x0D

unsigned int TXByte;
unsigned char BitCnt;
volatile long tempRaw;
volatile long tempSet = 0;
volatile int i;

unsigned int TxHI[]={0x48,0x49,0x0A,0x08,0x08};
unsigned int TxLO[]={0x4C,0x4F,0x0A,0x08,0x08};
unsigned int TxIN[]={0x49,0x4E,0x0A,0x08,0x08};

void FaultRoutine(void);
void ConfigWDT(void);
void ConfigClocks(void);
void ConfigPins(void);
void ConfigADC10(void);
void ConfigTimerA2(void);
void Transmit(void);

void main(void)
{
  ConfigWDT();
  ConfigClocks();
  ConfigPins();
  ConfigADC10();
  ConfigTimerA2();

  while(1)
  {
   _bis_SR_register(LPM3_bits + GIE);		// turn on interrupts and LPM3
   if (tempSet == 0)
   {
   	tempSet = tempRaw;	 					// Set reference temp
   }
   if (tempSet > tempRaw + 5)	 			// test for lo
   {
   	P1OUT = BIT6;		 					// green LED on
   	P1OUT &= ~BIT0;		 					// red LED off
   	for (i=0;i<5;i++)
   	 {
   	  TXByte = TxLO[i];
   	  Transmit();
   	 }
   }
   if (tempSet < tempRaw - 5)	 			// test for hi
   {
   	P1OUT = BIT0;		 					// red LED on
   	P1OUT &= ~BIT6;		 					// green LED off
   	for (i=0;i<5;i++)
   	 {
   	  TXByte = TxHI[i];
   	  Transmit();
   	 }
   }
   if (tempSet <= tempRaw + 2 & tempSet >= tempRaw - 2)
   {					 					// test for in range
   	P1OUT &= ~(BIT0 + BIT6); 				// both LEDs off
   	for (i=0;i<5;i++)
   	 {
   	  TXByte = TxIN[i];
   	  Transmit();
   	 }
    }
  }
}

void ConfigWDT(void)
 {
  WDTCTL = WDT_ADLY_250;      				// <1 sec WDT interval
  IE1 |= WDTIE;               				// Enable WDT interrupt
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
  BCSCTL2 = 0;               				// MCLK = DCO = SMCLK
 }

void FaultRoutine(void)
 {
   P1OUT = BIT0;                            // P1.0 on (red LED)
   while(1); 			                    // TRAP
 }

void ConfigPins(void)
 {
  P1SEL |= TXD + RXD;						// P1.1 & 2 TA0, rest GPIO
  P1DIR = ~(BIT3 + RXD);					// P1.3 input, other outputs
  P1OUT = 0;              					// clear output pins
  P2SEL = ~(BIT6 + BIT7); 					// P2.6 and 7 GPIO
  P2DIR |= BIT6 + BIT7;	  					// P1.6 and 7 outputs
  P2OUT = 0;		  						// clear output pins

 }

void ConfigADC10(void)
 {
   ADC10CTL1 = INCH_10 + ADC10DIV_0;        // Temp Sensor ADC10CLK
 }

void ConfigTimerA2(void)
  {
   CCTL0 = OUT;                             // TXD Idle as Mark
   TACTL = TASSEL_2 + MC_2 + ID_3;          // SMCLK/8, continuous mode

  }

// WDT interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void WDT(void)
{
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
  _delay_cycles(500);                       // Wait for ADC Ref to settle
  ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
  _delay_cycles(100);
  ADC10CTL0 &= ~ENC;				        // Disable ADC conversion
  ADC10CTL0 &= ~(REFON + ADC10ON);          // Ref and ADC10 off
  tempRaw = ADC10MEM;						// Read conversion value
  _bic_SR_register_on_exit(LPM3_bits); 		// Clr LPM3 bits from SR on exit
}

// Function Transmits Character from TXByte
void Transmit()
{
  BitCnt = 0xA;                     		// Load Bit counter, 8data + ST/SP
  while (CCR0 != TAR)               		// Prevent async capture
    CCR0 = TAR;                     		// Current state of TA counter
  CCR0 += Bitime;                   		// Some time till first bit
  TXByte |= 0x100;                  		// Add mark stop bit to TXByte
  TXByte = TXByte << 1;             		// Add space start bit
  CCTL0 =  CCIS0 + OUTMOD0 + CCIE;  		// TXD = mark = idle
  while ( CCTL0 & CCIE );           		// Wait for TX completion
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  CCR0 += Bitime;                   		// Add Offset to CCR0
  if (CCTL0 & CCIS0)                		// TX on CCI0B?
  {
    if ( BitCnt == 0)
    {
      CCTL0 &= ~ CCIE ;             		// All bits TXed, disable interrupt
    }

    else
    {
      CCTL0 |=  OUTMOD2;            // TX Space
      if (TXByte & 0x01)
      CCTL0 &= ~ OUTMOD2;           // TX Mark
      TXByte = TXByte >> 1;
      BitCnt --;
    }
  }
}
