/*
 * main.c
 */
#include "msp430fr5739.h"

#define UART_RXBot	BIT1			// RXD on P2.1
#define UART_TXBot	BIT0        	// TXD on P2.0
#define UART_RXSd	BIT6			// RXD on P2.6
#define UART_TXSd	BIT5        	// TXD on P2.5

void initClocks(void);						//initializes MCLK and ACLK
//void initUARTBot(void);					//initializes UART0
void initUARTSdCard(void);					//initializes UART1

int cnterTest = 0;
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";


int i = 0;

void main(void) {

	initClocks();
	initUARTSdCard();
	initLEDs();

	storeDataFlag = 1;

	_EINT();

	while(1){
		blinkLED3(1);
	}

}

void initClocks(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	// XT1 Setup
	PJSEL0 |= BIT4 + BIT5;

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELA_0 + SELS_3 + SELM_3;        // set ACLK = XT1; MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
	CSCTL4 |= XT1DRIVE_0;
	CSCTL4 &= ~XT1OFF;
	// CSCTL0_H = 0x01;                          // Lock Register

	do
	{
		CSCTL5 &= ~XT1OFFG;
		// Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
}

void initUARTSdCard(void)
{
	// Configure UART 1
	UCA1CTL1 |= UCSWRST;
	UCA1CTL1 = UCSSEL_2;                    	// Set SMCLK = 8MHz as UCBRCLK
	UCA1BR0 = 52;                 				// (8000000Hz / 9600bps) / 16  = 52.083
	UCA1BR1 = 0;
	UCA0MCTLW |= UCOS16;						// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	UCA0MCTLW |= 0x01;						// 52.083-52 = 0.083 *16 = 1.33
	UCA0MCTLW |= 0x49;                      	// UCBRSx value 0.3335= 0x42 (See UG)
	//UCA0MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)
	UCA1CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= UART_RXSd + UART_TXSd;
	P2SEL0 &= ~(UART_RXSd + UART_TXSd);

	UCA1IE |= UCRXIE;                         	// Enable RX interrupt
}

/*void initUARTBot(void)
{
	// Configure UART 0
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK = 8MHz as UCBRCLK
	UCA0BR0 = 52;                           	// (8000000Hz / 9600bps) / 16  = 52.083
	UCA0BR1 = 0;
	//UCA0MCTLW |= UCOS16;						// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	//UCA0MCTLW |= 0x01;						// 52.083-52 = 0.083 *16 = 1.33
	//UCA0MCTLW |= 0x49;                      	// UCBRSx value 0.3335= 0x42 (See UG)
	UCA0MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)

	UCA0CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= UART_RXBot + UART_TXBot;
	P2SEL0 &= ~(UART_RXBot + UART_TXBot);

	UCA0IE |= UCRXIE;                         	// Enable RX interrupt
}*/

/*
// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x08))		// Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG
		PJOUT^= BIT0;
		while (!(UCTXIFG & UCA0IFG));         // USCI_A0 TX buffer ready?
		UCA0TXBUF = UCA0RXBUF;                // TX -> RXed character
		break;

	case 4:break;                             // Vector 4 - TXIFG
	default: break;
	}
}*/

// Receives data from the SD card
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,0x08))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		PJOUT^= BIT0;
		while (!(UCTXIFG & UCA1IFG));           // USCI_A1 TX buffer ready?
		UCA1TXBUF = UCA1RXBUF;                // TX -> RXed character

		break;
	case 4:break;                             // Vector 4 - TXIFG
	default: break;
	}
}


