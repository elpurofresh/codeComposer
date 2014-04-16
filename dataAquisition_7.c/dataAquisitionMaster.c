//#include  "msp430g2553.h"

//#define LED1		BIT0
//#define UART_RXD	BIT1			// RXD on P1.1
//#define UART_TXD	BIT2        	// TXD on P1.2


volatile unsigned int PH=0;
volatile unsigned int Ox=0;
volatile unsigned int temp=0;
//unsigned char bufferRX;				

unsigned int writeProcess = 9;

unsigned int initialize=2;

//void setupUART(void);			//initialize the UART 
void setupBUTTONS(void);		//initialize button commands
//void blinkLED1(void);			//blink notification and delay
//void UART_putChar (char c);		//load the TX buffer

void main(void)
{
	setupUART();				//initialize UART
	setupBUTTONS();				//initialize button commands

	int writeProcess = 1;		//Enable (this will have to be adjusted in final code)
	_EINT();					//enable interrupts

	while(writeProcess){		

		if (initialize == 1) {	//Sets Slave Clock time
			blinkLED1();
			UART_putChar('F');
		}
		
		if (initialize==0) {	//Get's time stamp from the slave
			blinkLED1();
			UART_putChar('G');
			blinkLED1();
			PH=bufferRX;
			blinkLED1();
			Ox=bufferRX;
			blinkLED1();
			temp=bufferRX;
		}
			initialize = 2;	
	}
}
/*
void setupUART(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
	P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
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
*/


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
/*
void blinkLED1(void)
{
	volatile unsigned int j=25000;

	P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
	do j--;						// Make some delay
	while (j != 0);

	P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
}*/

/**
 * Send only byte via UART.
 *
 * @param c Character to send.
 */
 
 /*
void UART_putChar (char charToSend)              // UART1 Transmit Subroutine
{
	IFG2 &= ~UCA0RXIFG;
	while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data
	//bufferRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

}
*/
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
