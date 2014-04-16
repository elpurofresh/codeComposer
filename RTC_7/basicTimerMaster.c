//master
//******************************************************************************
//   MSP430G2xx3 Demo - USCI_A0, Ultra-Low Pwr UART 9600 String, 32kHz ACLK
//
//   Description: This program demonstrates a full-duplex 9600-baud UART using
//   USCI_A0 and a 32kHz crystal.  The program will wait in LPM0, and will
//   respond to a received 'u' character using 8N1 protocol. The response will
//   be the string 'Hello World'.
//   ACLK = BRCLK = LFXT1 = 32768Hz, MCLK = SMCLK = DCO ~1.2MHz
//   Baud rate divider with 32768Hz XTAL @9600 = 32768Hz/9600 = 3.41
//* An external watch crystal is required on XIN XOUT for ACLK *//
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 | 32kHz
//          --|RST          XOUT|-
//            |                 |
//            |     P1.2/UCA0TXD|------------>
//            |                 | 9600 - 8N1
//            |     P1.1/UCA0RXD|<------------
//
//   D. Dang
//   Texas Instruments Inc.
//   February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include  "msp430g2553.h"

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define LED1		BIT0
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define RESET_PIN	BIT4			// Reset line on P1.4

unsigned char bufferRX;				

unsigned int writeProcess = 9;

unsigned int initialize=2;

//values for clock update.  for testing only
int sec = 0;

int secStamp=0;
volatile unsigned int i;

void setupUART(void);			//initialize the UART 
void setupBUTTONS(void);		//initialize button commands
void blinkLED1(void);			//blink notification and delay
void UART_putChar (char c);		//load the TX buffer
void checkStatus(void);
void sleep(void);

void main(void)
{
	setupUART();				//initialize UART
	setupBUTTONS();				//initialize button commands
	checkStatus();

	int writeProcess = 1;		//Enable (this will have to be adjusted in final code)
	_EINT();					//enable interrupts

	while(writeProcess){		

		if (initialize == 1) {	//Sets Slave Clock time
			blinkLED1();
			UART_putChar('F');
			blinkLED1();
			UART_putChar(sec);
		}
		
		if (initialize==0) {	//Get's time stamp from the slave
			blinkLED1();
			UART_putChar('G');
			blinkLED1();
			secStamp=bufferRX;
		}
			initialize = 2;	
	}
}

void setupUART(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
	P1DIR = 0xFF & ~(UART_RXD + BIT3 + BIT5);                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                 // All P1.x reset
	P1SEL = UART_RXD + UART_TXD;              // P1.1 = RXD, P1.2=TXD
	P1SEL2 = UART_RXD + UART_TXD;             // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void setupBUTTONS(void){
	//port 1
	P1IE |= BIT3;							//Port 1 BIT3 interrupt enabled
	P1IES |= BIT3;							//Port 1 Hi/Lo trigger edge
	P1IFG &= ~BIT3;							//clear port 1 interrupt flag
	//port 2
	P2IE |= BIT0;							//Port 2 BIT3 interrupt enabled
	P2IES |= BIT0;							//Port 2 Hi/Lo trigger edge
	P2IFG &= ~BIT0;							//clear port 1 interrupt flag
	
}

void blinkLED1(void)
{
	volatile unsigned int j=25000;

	P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
	do j--;						// Make some delay
	while (j != 0);

	P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}

void sleep(void)
{
	volatile unsigned int j=25000;

	//P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
	do j--;						// Make some delay
	while (j != 0);

	//P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}

/**
 * Send only byte via UART.
 *
 * @param c Character to send.
 */
void UART_putChar (char charToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data
	//bufferRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}

void checkStatus(void){
	P1OUT |= BIT4;
	P1OUT &= ~BIT5;
	
	UART_putChar('C');
	
	sleep();
	
	while(bufferRX != 'K'){
			for(i=0; i<4; i++){
				P1OUT ^=BIT0;
				_delay_cycles(100000);
			}
			_delay_cycles(500000);
		}
	
	UART_putChar('O');
	
	for(i=0; i<2; i++){
		P1OUT ^= BIT6;
		_delay_cycles(10000000);
		}
	}
//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	//UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character  Echoes received byte
	bufferRX = UCA0RXBUF;                     // Store RXed character
}

#pragma vector=PORT1_VECTOR
__interrupt void port_1(void){
	initialize = 1;
	P1IES ^= BIT3;
	P1IFG &= ~BIT3;
}

#pragma vector=PORT2_VECTOR
__interrupt void port_2(void){
	initialize = 0;
	P2IES ^= BIT0;
	P2IFG &= ~BIT0;
}
