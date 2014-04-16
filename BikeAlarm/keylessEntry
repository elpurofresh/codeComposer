/*
 * main.c
 */
#include "msp430g2553.h"

#define TRUE		1				// Equivalent to boolean 'true'
#define FALSE		0				// Equivalent to boolean 'false'
#define RED_LED		BIT0
#define GREEN_LED	BIT6
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define HALL_SNR	BIT5			// Hall sensor on P1.5
#define STX_BYTE	0x02			// Refer to STX = Start of Text www.asciitable.com
#define ETX_BYTE	0x03			// Refer to ETX = End of Text www.asciitable.com
#define dataSize	13				// STX + 10 ASCII Data Chars + ChkSum + ETX + \n

//char password[dataSize]		= {STX_BYTE, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, ETX_BYTE, '\n'};
const char password[dataSize]		= "4D000E410C0E\n";
char dataPacket[dataSize] 	= "$$$$$$$$$$$$\n";
unsigned int pcktStartFlag	= 0;	// Flags start of packet
unsigned int packetRxd		= 0;	// If packet successfully rx'd -> TRUE
unsigned int packetCnter	= 0;	// Keeps count of data packet's bytes number to avoid overflow
unsigned int timerCount		= 0;
unsigned int timeToStop		= 0;


void setupClock(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
}

void setupPorts(void)
{
	P1DIR = 0xFF & ~(UART_RXD + HALL_SNR);		// All P1.x outputs except UART receiving pin
	P2DIR = 0xFF;								// All P2.x ports outputs
	P1IN |= UART_RXD;							// Port P1.1 is enabled/set as an input
	P1OUT = 0;                                	// All P1.x reset
	P1SEL = UART_RXD + UART_TXD;              	// P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD + UART_TXD;             	// P1.1 = RXD, P1.2=TXD

	P1OUT &= ~HALL_SNR;							// P1.5 set to pull-up (input port)
	P1IES |= HALL_SNR;							// P1.5 Lo/Hi edge interrupt
	P1IE  |= HALL_SNR;							// P1.5 interrupt enabled
	P1IFG &= ~HALL_SNR;							// P1.5 IFG cleared
}

void setupUART(void)
{
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void setupTimer(void)
{
	TA0CCR0 = 32768-1;							// Count from 0-32767
	//TA0CCTL0 |= CCIE;							// Enable clock interrupt
	TA0CTL = TASSEL_1 + MC_1 + TACLR;		// Start Timer from ACLK(XTAL@32.768kHZ), Up Mode, Divide 32kHZ by 8, Clear
}

void blinkLEDRed(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2;  // max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = max; i > 0; i--) {
		P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
}

void blinkLEDGreen(int numOfBlinks)
{
	volatile unsigned int j=25000;
	int i = 0, max = numOfBlinks*2; //max is multiplied by two since toggling only gives half the On/Off cycle

	for (i = max; i > 0; --i) {
		P1OUT ^= BIT6;              // Toggle P1.0 using exclusive-OR
		do j--;						// Make some delay
		while (j != 0);
	}
}

int processPacket(void)
{
	int i = 0;

	for (i = 0; i < dataSize; ++i) {
		if (dataPacket[i] != password[i]) {
			return FALSE;
		}
	}

	return TRUE;
}

int keepLocksOpen(void)
{
	if (timerCount > 9) {
			timerCount = 0;
			return FALSE;
		}
	return TRUE;
}

void openLocks(void)
{
	P2DIR |= BIT0 + BIT1;
	P2OUT |= BIT0 + BIT1;

}

void closeLocks(void)
{
	P2OUT &= ~BIT0 & ~BIT1;
}

void delaySecs(int secs)
{
	TA0CCR0 = 32768-1;		// Reset the counter register to go up to 32768-1
	TACCTL0 |= CCIE;		// Enable the interrupt

	while (timerCount < secs)
	{
		//do nothing
	}

	TA0CCR0 = 0;
	TA0CCTL0 &= ~CCIE;

}

void main(void) {

	setupClock();
	setupPorts();
	setupUART();
	setupTimer();

	_EINT();

	closeLocks();

	while(TRUE)
	{
		P1OUT |= RED_LED;
		delaySecs(10);
		P1OUT &= ~RED_LED;

		/*if (packetRxd == TRUE) {

			if (processPacket() == TRUE) {
				openLocks();
				blinkLEDGreen(3); //keeps loop 10 secs here, effectively making the loop wait
				packetRxd = FALSE;
				closeLocks();
			}
		} else {
			closeLocks();
		}*/
	}
}


// Interrupt in charge of the UART Module
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?

	if (!pcktStartFlag && UCA0RXBUF == STX_BYTE) {
		pcktStartFlag = TRUE;
		packetCnter = 0;
		packetRxd = FALSE;
	}

	else if (pcktStartFlag && UCA0RXBUF != ETX_BYTE) {
		dataPacket[packetCnter++] = UCA0RXBUF;
	}
	else if (pcktStartFlag && UCA0RXBUF == ETX_BYTE) {
		pcktStartFlag = FALSE;
		packetRxd = TRUE;
	}
}

//TimeA0 interrupt (For timing purpuses
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	timerCount++;
	/*if (timerCount > timeToStop) {
		TA0CCR0 = 0;
		TA0CCTL0 &= ~CCIE;
	}*/

	__low_power_mode_off_on_exit();    // Restores active mode on return
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{

	blinkLEDRed(1);
	P1IFG &= ~HALL_SNR;         	// Clear P1.5 IFG

	__low_power_mode_off_on_exit();    // Restores active mode on return
}
