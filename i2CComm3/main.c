//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Slave RX single bytes from MSP430 Master
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the slave code. The interrupt driven
//  data receiption is demonstrated using the USCI_B0 RX interrupt.
//  ACLK = n/a, MCLK = SMCLK = default DCO = ~1.2MHz
//
//                                /|\  /|\
//               MSP430G2xx3      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P1.7/UCB0SDA|<-|---+->|P1.7/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P1.6/UCB0SCL|<-+----->|P1.6/UCB0SCL     |
//            |                 |         |                 |
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430g2553.h"

volatile unsigned char RXData = 'B';
volatile int sec = 0, min = 0, hrs = 0;

void setupTimer(void);
void setupPorts(void);
void timeToBroadcast(void);

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0I2COA = 0x48;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;                          // Enable RX interrupt

	_EINT();

	setupPorts();
	//setupTimer();

	while (1)
	{
		__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
		__no_operation();                       // Set breakpoint >>here<< and read
	}                                         // RXData
}

void setupTimer(void)
{
	TA0CCTL0 = CCIE;							// Enable clock interrupt
	TA0CCR0 = 32768-1;							// Count from 0-32767
	TA0CTL = TASSEL_1 + MC_1 + TACLR;			// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, clear
}

void setupPorts(void)
{
	P1DIR |= BIT0 + BIT4;						// P1.4 set to ouput
	P1OUT &= ~(BIT0+ BIT4);						// P1.4 set to Low
}

void timeToBroadcast(void)
{
	P1OUT |= BIT4;
	_delay_cycles(250000);
	P1OUT &= ~BIT4;
}

/*#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	P1OUT ^= BIT0;
	sec++;
	if (sec == 59) { sec = 0; min ++; }
	if ((sec % 5) == 0) {

		TA0CCTL0 &= ~ CCIE;
		timeToBroadcast();
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }

	P1OUT ^= BIT0;


	//__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return
}*/

// USCI_B0 Data ISR
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	P1OUT ^= BIT0;
	RXData = UCB0RXBUF;                       // Get RX data
	//TA0CCTL0 = CCIE;
	__bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
