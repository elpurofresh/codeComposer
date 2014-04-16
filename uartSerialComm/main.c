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

unsigned int enterCmdModeSent = 0;
unsigned int newFileCmdSent = 0;
unsigned int apndDataCmdSent = 0;
unsigned char bufferRX; // = '+';
char processFlag = 0;
char test[] = "Hola fonola";


void resetSDCard (void);
void setupUART(void);
void enterSDCardCmdMode(void);
void blinkLED1(void);
void UART_init (void);
void UART_putChar (char c);
char UART_getCharIsReady ();
char UART_getChar ();
void UART_write (const char *buf);
void writeSensorData(void);

void main(void)
{
	//WDTCTL = WDTPW + WDTHOLD;               // Stop WDT
	//P1DIR |= LED1;               			// Set P1.0 to output direction
	//P1OUT &= ~LED1;							// Set P1.0 to LOW, LED off

	//resetSDCard();
	setupUART();

	_EINT();
	//__bis_SR_register(LPM0_bits + GIE);       //If this is ON the while loop does not enter Enter LPM0 w/interrupts enabled until Byte RXed


	//enterSDCardCmdMode();



	while(1){
		//blinkLED1();
		//bufferRX = UART_getChar();
		//UART_putChar(bufferRX);

		/*if (bufferRX == '<') {
			blinkLED1();
			//UART_putChar('X');
			writeSensorData();
			//bufferRX = 'N'; //flush the buffer to avoid getting in a endless echo loop
		}*/

		if (bufferRX == '<' && apndDataCmdSent == 0) {
			enterSDCardCmdMode();
			enterCmdModeSent = 1;
		}

		if (bufferRX == '>' && enterCmdModeSent == 1) {
			UART_write("new DATA100.txt");
			newFileCmdSent = 1;
			enterCmdModeSent = 0;
		}

		if (bufferRX == '>' && newFileCmdSent == 1) {
			UART_write("append DATA100.txt");
			apndDataCmdSent = 1;
			newFileCmdSent = 0;
		}

		if (bufferRX == '<' && apndDataCmdSent == 1) {
			writeSensorData();
			apndDataCmdSent = 0;
		}
	}

}

void resetSDCard (void){
	//P1DIR &= ~UART_TXD;  	// OpenLog Rx to GND, msp430TX to input
	//P1IN &= ~UART_TXD;   	// MSP430TX to LOW

	P1DIR |= RESET_PIN;   	// MSP430 P1.4 to output
	P1OUT &= ~RESET_PIN;   	// MSP430 P1.4 to LOW


	__delay_cycles(100);  	// Give some time to SD Card
	blinkLED1();			// Give some indication

	P1OUT |= RESET_PIN;  	// MSP430 P1.4 to HIGH

	//__delay_cycles(10);  // Give some time to SD Card

	//P1DIR |= BIT4;   	// MSP430 P1.4 to output
	//P1OUT |= BIT4;   	// MSP430 P1.4 to HIGH

	//P1DIR = 0XFF;	 	// ALL Ports 1 to OUTPUT
	//P1OUT = 0;		// RESET ALL Ports 1
}

void setupUART(void)
{
	/*P1DIR = 0xFF & ~UART_RXD;                 // All P1.x outputs
	P1IN |= UART_RXD;
	P1OUT = 0;                                // All P1.x reset
	P1SEL = UART_TXD + UART_RXD ;          	  // P1.1 = RXD, P1.2=TXD
	P2DIR = 0xFF;                             // All P2.x outputs
	P2OUT = 0;                                // All P2.x reset
	UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA0BR0 = 0x03;                           // 32kHz/9600 = 3.41
	UCA0BR1 = 0x00;                           //
	UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt*/

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


void enterSDCardCmdMode(void)
{
	int i;
	int numCmdsToSend = 3;
	for (i = 0; i < numCmdsToSend; ++i) {
		UART_putChar(CTRLZ);
	}
	UART_putChar(CR);
}

void blinkLED1(void)
{
	volatile unsigned int j=25000;

	P1OUT ^= BIT0;              // Toggle P1.0 using exclusive-OR
	do j--;						// Make some delay
	while (j != 0);

	P1OUT &= ~BIT0;					// Forces the LED at P1.0 to turn OFF at the end of the routine
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
	bufferRX = '#';				 //flush the buffer to avoid getting in a endless echo loop

	//while ((UCA0TXIFG & IFG2) == 0) { /* Wait for the byte to be sent */
	//blinkLED2();
	//}
}

/**
 * Check UART's input buffer.
 *
 * @return TRUE: if input buffer contains received data.
 */
char UART_getCharIsReady ()
{
	if (IFG2 & UCA0RXIFG) {
		IFG2 &= ~UCA0RXIFG;  // Clear Flag UCA0RXIFG since it is not automatically cleared
		return 1;
	} else {
		return 0;
	}

	//return (IFG2 & UCA0RXIFG) != 0;

}

/**
 * Receive one byte via UART. This function waits for the input (blocking).
 *
 * @return Received character.
 */
char UART_getChar ()
{
	while (!UART_getCharIsReady ());
	return UCA0RXBUF;
}

/**
 * Write string buffer to UART.
 *
 * @param buf String buffer to send.
 * @param length Length of string buffer.
 */
void UART_write (const char *buf)
{
	int i;
	//int bufSize = sizeof(buf);
	for (i = 0; i < strlen(buf); i++)
	{
		UART_putChar (buf[i]);
	}

	UART_putChar(CR);
}

void writeSensorData(void)
{
	UART_write("t100,ph4,cd200");
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	//UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character  Echoes received byte
	bufferRX = UCA0RXBUF;                     // Store RXed character
}

/*#pragma vector=USCIAB0RX_VECTOR
__interrupt void USART0_RX (void)
{
	buffer[bufpos] = UCA0RXBUF;
	if (bufpos == (MaxSizeOfBuffer-1) || buffer == "12<") {
		processFlag = 1;
		bufpos = 0;
	}
}*/

/*#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF = dummyData[i++];                 // TX next character

	if (i == sizeof dummyData - 1)           	// TX over?
		IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
}*/

/*#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	bufferRX = UCA0RXBUF;
	P1OUT ^= BIT0;					// Toggle LED at P1.0							//


	if (UCA0RXBUF == '<')                     // '<' received?
	{
		P1OUT |= BIT0;                          // Turn ON LED
		_delay_cycles(1000);
		P1OUT &= ~BIT0;							// Turn OFF LED

		i = 0;
		IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
		UCA0TXBUF = dummyData[i++];
		//UCA0TXBUF = stringToSend("newFileCmd", i);
	}
}*/
