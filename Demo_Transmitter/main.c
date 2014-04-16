#include "msp430fr5739.h"

char uartBuffer[20]="$$$$$$$$$$$$$$$$$$$$";
int address = 0;


void main(void){

	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;   // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELS_3 + SELM_3;        // set ACLK = XT1; MCLK = DCO
	CSCTL3 = DIVS_0 + DIVM_0;        // set all dividers

	// Configure UART 0
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK = 8MHz as UCBRCLK
	UCA0BR0 = 52;                           	// (8000000Hz / 9600bps) / 16  = 52.083
	UCA0BR1 = 0;
	// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	// 52.083-52 = 0.083 *16 = 1.33
	// UCBRSx value 0.3335= 0x42 (See UG)
	UCA0MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)

	UCA0CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= BIT1 + BIT0;						//RXD = 2.1
	P2SEL0 &= ~(BIT1 + BIT0);					//TXD = 2.0

	UCA0IE |= UCRXIE;                         	// Enable RX interrupt
	_EINT();
	
	while(1){


	}
}

// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))	  // Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		UCA0TXBUF = UCA0RXBUF;

		uartBuffer[address]=UCA0RXBUF;
		address++;

		if(address>=20){
			address=0;
		}

		break;

	default: break;
	}

	__low_power_mode_off_on_exit();
}

