//******************************************************************************
//  MSP430x22x4 Demo - USCI_B0 I2C Slave RX/TX multiple bytes to MSP430 Master
//
//  Description: This demo connects two MSP430's via the I2C bus. The slave
//  receives then transmits to the master. This is the slave code. The interrupt
//  driven data transmission is demonstrated using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = default DCO = ~1.045Mhz
//
//	***to be used with msp430x22x4_uscib0_i2c_12.c***
//
//                                /|\  /|\
//               MSP430G2xx3       10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P3.1/UCB0SDA|<-|---+->|P3.1/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P3.2/UCB0SCL|<-+----->|P3.2/UCB0SCL     |
//            |                 |         |                 |
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430g2553.h"


#define NUM_BYTES  1 //2                        // How many bytes?
//**** Please note this value needs to be the same as NUM_BYTES_RX in the
//     associated master code. This definition lets the slave know when to
//     switch from RX interrupt sources to TX interrupt sources. This is 
//     important since the interrupt vectors are shared by TX and RX flags.

unsigned char *PTxData;                     // Pointer to TX data
unsigned char *PRxData;                     // Pointer to RX data
volatile unsigned char RxBuffer[NUM_BYTES];       // Allocate 128 byte of RAM
//char SLV_Data = 0x11;
volatile unsigned char TXByteCtr, RXByteCtr, RX = 0;
//volatile unsigned char RxBuffer[NUM_BYTES];       // Allocate 128 byte of RAM
char dataToSend[6] = "235910";

volatile int sec = 0, min = 0, hrs = 0;


void USCI_SLAVE_SETUP(void);
void Setup_RX(void);
void Receive(void);

void setupTimer(void);
void setupPorts(void);
void timeToBroadcast(void);

void blinkLED1(int numOfBlinks);

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	setupPorts();
	//setupTimer();

	_EINT();

	while(1){

		blinkLED1(1);
		USCI_SLAVE_SETUP();

	}
}

void setupPorts(void)
{
	P1DIR |= BIT0 + BIT4;		// P1.4 set to ouput
	//P1REN |= BIT4;			// P1.4 pull-up set on it
	P1OUT &= ~(BIT0+ BIT4);			// P1.4 set to Low
	//P1IES &= ~BIT4;			// P1.4 Lo/Hi edge interrupt
	//P1IE |= BIT4;			// P1.4 interrupt enabled
	//P1IFG &= ~BIT4;			// P1.4 IFG cleared
}

void blinkLED1(int numOfBlinks)
{
	int i = 0, max = numOfBlinks*2; 	//max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = 0; i < max; ++i) {
		P1OUT ^= BIT0;             	// Toggle P1.0 using exclusive-OR
		__delay_cycles(500000);
	}
}

void USCI_SLAVE_SETUP(void){
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	Setup_RX();
	Receive();
}

void Setup_RX(void){
	_DINT();
	RX = 1;
	IE2 &= ~UCB0TXIE;                         // Disable TX interrupt
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0I2COA = 0x48;                         // Own Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	UCB0I2CIE |= UCSTPIE + UCSTTIE;           // Enable STT and STP interrupt
	IE2 |= UCB0RXIE;                          // Enable RX interrupt

	blinkLED1(1);
}

void Receive(void){
	PRxData = (unsigned char *)RxBuffer;    // Start of RX buffer
	RXByteCtr = 0;                          // Clear RX byte count
	TXByteCtr = 0;

	blinkLED1(1);

	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
	// Remain in LPM0 until master
	// finishes TX
}

void setupTimer(void)
{
	TA0CCTL0 = CCIE;							// Enable clock interrupt
	TA0CCR0 = 32768-1;							// Count from 0-32767
	TA0CTL = TASSEL_1 + MC_1 + TACLR;			// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, clear
}


void timeToBroadcast(void)
{
	P1OUT |= BIT4;
	_delay_cycles(250000);
	P1OUT &= ~BIT4;
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move data from MSP430 memory to the
// I2C master. PTxData points to the next byte to be transmitted, and TXByteCtr
// keeps track of the number of bytes transmitted.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	if(RX == 0) {
		/*UCB0TXBUF = SLV_Data++;      		// Transmit data at address PTxData
		TXByteCtr++;						// Increment TX byte counter*/

		//UCB0TXBUF = dataToSend[TXByteCtr];
		UCB0TXBUF = UCB0RXBUF;
		TXByteCtr++;						// Increment TX byte counter

		if (TXByteCtr >= NUM_BYTES) {
			IE2 &= ~UCB0TXIE;
			IE2 |= UCB0RXIE;
			TXByteCtr = 0;

		}
	}

	if(RX == 1){
		*PRxData++ = UCB0RXBUF;       		// Move RX data to address PRxData
		RXByteCtr++;						// Increment RX byte count

		if(RXByteCtr >= NUM_BYTES) {		// Received enough bytes to switch
			RX = 0;							// to TX?
			IE2 &= ~UCB0RXIE;
			IE2 |= UCB0TXIE;
			RXByteCtr = 0;
			//TA0CCTL0 |= CCIE;
		}
	}
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to do
// processing in the main program after data has been transmitted. LPM0 is
// only exit in case of a (re-)start or stop condition when actual data
// was transmitted.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{ 
	blinkLED1(1);

	if(RX == 0){
		UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);			// Clear interrupt flags

		if (TXByteCtr) {							// Check TX byte counter
			__bic_SR_register_on_exit(CPUOFF);		// Exit LPM0 if data was
		}											// transmitted
	}

	if(RX == 1){
		UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);			// Clear interrupt flags
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	P1OUT ^= BIT0;
	sec++;
	if (sec == 59) { sec = 0; min ++; }
	if ((sec % 5) == 0) {

		//TA0CCTL0 &= ~ CCIE;
		timeToBroadcast();
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }

	P1OUT ^= BIT0;


	//__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return
}
