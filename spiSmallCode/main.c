
//#include <msp430x16x.h>
#include <msp430g2553.h>

void blinkLED(int numOfBlinks);
void init_SPI_Module();
void spi_cs_assert();
void spi_cs_deassert();
void spi_send_byte(unsigned char input);
unsigned char spi_rcv_byte(void);
void spi_send_multipleDummyData();

#define DUMMYDATA 0xFF
#define GoIdleState 0x40
#define CMDIdle	0x01
#define APP_CMD 0x77
#define R1_ILLEGAL_CMD 0x04
#define APP_SEND_OP_COND 0x69

unsigned char tmpRX;
unsigned int status = 0;
int numOfTries = 0;
int MAX_NumOfTries = 50;
int initTryDummyData = 10;
int cardTypeIsSdCard = 0;
int cardIsInitialized = 0;

void main (){

	int i;

	WDTCTL = WDTPW + WDTHOLD; // disable watch dog for testing

	P1OUT = 0x00;                          	// P1.0 setup for LED &
	P1DIR |= BIT0 + BIT5;                   // P1.5 reset output (Chip Select)
	P1SEL = BIT1 + BIT2 + BIT4;				//
	P1SEL2 = BIT1 + BIT2 + BIT4;			//

	init_SPI_Module();						//Initialize SPI Module


	while (status !=0  && cardIsInitialized != 1 && numOfTries < MAX_NumOfTries) {
		spi_cs_deassert();
		spi_send_multipleDummyData();

		spi_cs_assert();
		spi_send_byte(GoIdleState);

		__delay_cycles(75);

		for (i = 0; i < initTryDummyData; i++) {

			//if (spi_rcv_byte() == CMDIdle) {
			if (tmpRX == CMDIdle) {
				spi_cs_deassert();
				spi_send_byte(DUMMYDATA);
				status = 1;
				blinkLED(5);
			}
		}

		spi_cs_assert();
		spi_send_byte(APP_CMD);
		if (/*spi_rcv_byte()*/tmpRX != R1_ILLEGAL_CMD) { /*If condition true it is an SD card*/
			cardTypeIsSdCard = 1;
			spi_cs_deassert();
			blinkLED(5);
		}

		spi_cs_assert();
		spi_send_byte(APP_SEND_OP_COND);
		spi_cs_deassert();

		spi_cs_deassert();
		spi_send_byte(DUMMYDATA);
		if (spi_rcv_byte() != 0x01) {
			cardIsInitialized = 1;
		}

		blinkLED(3);
		numOfTries++;
	}



}

void blinkLED (int numOfBlinks) {
	int i;
	P1DIR |= BIT0;
	_delay_cycles(100);

	for (i = 0; i < numOfBlinks; ++i) {
		P1OUT |= BIT0;                          // Turn ON LED
		_delay_cycles(1000);
		P1OUT &= ~BIT0;							// Turn OFF LED
	}
}

void init_SPI_Module() {
	UCA0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 |= 0x02;                          // /2
	UCA0BR1 = 0;                              //
	UCA0MCTL = 0;                             // No modulation
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt
}

void spi_cs_assert() {
	P1OUT &= ~BIT5; //0x00;
}

void spi_cs_deassert() {
	P1OUT |= BIT5; //0x00;
}


/* Send a single byte over the SPI port */
void spi_send_byte(unsigned char input)
{
	IFG2 &= ~UCA0RXIFG;
	while (!(UCA0TXIFG & IFG2));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = input;

	while ((UCA0TXIFG & IFG2) == 0) { /* Wait for the byte to be sent */
		blinkLED(3);
	}
}

/* Receive a byte. Output an 0xFF (the bus idles high) to receive the byte */
unsigned char spi_rcv_byte()
{
	unsigned char tmp;
	IFG2 &= ~UCA0RXIFG;
	UCA0TXBUF = DUMMYDATA;/* Send the byte */

	/* Wait for the byte to be received */
	while ((UCA0RXIFG & IFG2) == 0) { /* Wait for the byte to be sent */
		blinkLED(3);
	}
	tmp = UCA0RXBUF;
	return (tmp);
}

void spi_send_multipleDummyData()
{
	int i;
	for (i = 0; i < initTryDummyData; ++i) {
		spi_send_byte(DUMMYDATA);
	}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIA0RX_ISR(void)
{
	volatile unsigned int i;

	while (!(IFG2 & UCA0TXIFG));              // USCI_A0 TX buffer ready?

	tmpRX = UCA0RXBUF;

	__delay_cycles(50);                     // Add time between transmissions to
}
