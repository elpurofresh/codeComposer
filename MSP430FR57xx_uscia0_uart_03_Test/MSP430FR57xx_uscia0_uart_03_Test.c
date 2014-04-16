//******************************************************************************
//  MSP430FR57xx Demo - USCI_A0 UART echo at 9600 baud
//
//  Description: This demo echoes back characters received via a PC serial portt.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//  The example code shows proper initialization of registers 
//  and interrupts to receive and transmit data.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1MHz
//
//                                
//                MSP430FR5739 
//                       
//             -----------------   
//       RST -|     P2.0/UCA0TXD|----> PC (echo)
//            |                 |    
//           -|                 |    
//            |     P2.1/UCA0RXD|<---- PC
//            |                 |  
//
//   P. Thanigai
//   Texas Instruments Inc.
//   August 2010
//   Built with CCS V4 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430fr5739.h"
#include <string.h>

#define UART_RXBot	BIT1		// RXD on P2.1
#define UART_TXBot	BIT0        // TXD on P2.0
#define TRUE		1
#define FALSE		0
#define START_BYTE  0x5B		// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D		// Refer to ']' in ASCII Matrix
#define END_TX	0xD 			// '\n'
#define BUFF_SIZE	37			// Max buffers size= data + internal delimiter '\0'

void startCLK(void);
void stopCLK(void);
void startRx(void);
void stopRx(void);
void resetInputBuffer(void);
void txPacket(char *buffer);
void uart_PutCharOut(char charToSend);

void sleepQuart(void);						//delays the processor for 1/4 sec

volatile int cnter1 = 0, cnter2 = 0;
char uartRxBuff[BUFF_SIZE+2] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0"; //includes brackets [<-data->]
char uart_Out[BUFF_SIZE] 	= "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0";
char uartIn[BUFF_SIZE] 		= "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0";
volatile int rxd = FALSE, timeFlag = FALSE;
volatile int startFlag = FALSE;
volatile int dataLength = 0, clkTimer = 0, testCounter = 0;
volatile int flagACKTx = FALSE, flagRxMode = FALSE, firstTime = TRUE;


void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELS_3 + SELM_3;        // set ACLK = SMCLK; MCLK = DCO
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
	P2SEL1 |= UART_RXBot + UART_TXBot;
	P2SEL0 &= ~(UART_RXBot + UART_TXBot);

	P1DIR |= 0xFF; //(BIT0 + BIT1 + BIT2 + BIT5);
	P1OUT &= 0; //~(BIT0 + BIT1 + BIT2 + BIT5);

	//TA0CCTL0 = CCIE;					// TACCR0 interrupt enabled
	TA0CCR0 = 65536-1;					// CCR0 counter
	TA0CTL = TASSEL_2 + ID_3 + MC_1;	// SMCLK, f(SMCLK)/8, UP mode
	TA0EX0 = TAIDEX_7;					// Input divider expansion (f(SMCLK)/x)/8

	startRx();
	//startCLK();
	_EINT();

	while(1){

		/*if (rxd == TRUE) {
			rxd = FALSE;
			//stopRx();
			txPacket(uart_Test);
			//startRx();
		}*/

		/*if (rxd) {
			//rxd = FALSE;
			gotIn1++;
			//timeFlag = FALSE;
			stopRx();
			_delay_cycles(2000000);
			//stopCLK();
			txPacket(uartIn);
			_delay_cycles(2000000);
			//startCLK();
			//resetInputBuffer();
			startRx();
		}*/

		/*if (timeFlag && !rxd) {
			gotIn1++;
			timeFlag = FALSE;
			stopCLK();
			stopRx();
			uartRxBuff[cnter1+1] = '\0';
			_delay_cycles(4000000);
			txPacket(uartRxBuff);
			_delay_cycles(4000000);
			startRx();
		} else if (rxd) {
			gotIn2++;
			rxd = FALSE;
			stopCLK();
			stopRx();
			_delay_cycles(4000000);
			txPacket(uartIn);
			_delay_cycles(4000000);
			startRx();
		}*/

		/*if (!rxd) {
			gotIn1++;
			stopRx();
			uartRxBuff[cnter1+1] = '\0';
			txPacket(uartRxBuff);
			startRx();
		} else if (rxd) {
			gotIn2++;
			rxd = FALSE;
			stopRx();
			txPacket(uartIn);
			startRx();
		}*/


		//Receiver
		/*if (flagACKTx && flagRxMode) {
			txPacket("<\0");
			flagRxMode = FALSE;
		}

		if (!flagACKTx && flagRxMode) {
			txPacket(">\0");
		}

		//Transmitter
		if (flagACKTx && !flagRxMode) {
			//txPacket("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\0");
			uartIn[dataLength] = '\0';
			txPacket(uartIn);
			txPacket("?\0");
			flagACKTx = FALSE;
		}*/

		if (rxd && timeFlag) {
			rxd = FALSE;
			timeFlag = FALSE;
			stopCLK();
			stopRx();
			uartIn[BUFF_SIZE-1] = '\0';
			txPacket(uartIn);
			resetInputBuffer();
			_delay_cycles(8000000);
			startRx();
			firstTime = TRUE;
		}

		_low_power_mode_3();
	}
}

void sleepQuart(void)
{
	_delay_cycles(2000000);
}

void startRx(void)
{
	UCA0IE |= UCRXIE;                         	// Enable RX interrupt
}

void stopRx(void)
{
	UCA0IE &= ~UCRXIE;                         	// Enable RX interrupt
}

void startCLK(void)
{
	TA0CCTL0 |= CCIE;
	TA0CCR0 = 65536-1;
}

void stopCLK(void)
{
	TA0CCTL0 &= ~CCIE;
	TA0CCR0 = 0;
	clkTimer = 0;
}

void resetInputBuffer(void)
{
	int i = BUFF_SIZE;
	cnter1 = 0;
	for (i = BUFF_SIZE; i > 1; --i) {
		uartIn[i] = '@';
	}
	uartIn[BUFF_SIZE-1] = '\0';
}

void txPacket(char *buffer)
{
	static int i = 0;

	for (i = 0; i < strlen(buffer); ++i) {  // Using a down counter for optimization
		uart_PutCharOut(buffer[i]);
	}
}

void uart_PutCharOut(char charToSend)  	// UART0 Transmit Subroutine
{
	while (!(UCTXIFG & UCA0IFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data

	uart_Out[cnter2++] = UCA0TXBUF;

	if (cnter2 > BUFF_SIZE-1) {
		cnter2 = 0;
	}
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG
		uartRxBuff[cnter1++] = UCA0RXBUF;

		//if (UCA0RXBUF > 47 && UCA0RXBUF < 94) {
		rxd = TRUE;
		uartIn[dataLength++] = UCA0RXBUF;

		if (rxd && firstTime) {
			startCLK();
			firstTime = FALSE;
		}

		if (dataLength > BUFF_SIZE-1) {
			dataLength = 0;
		}

		/*//Receiver
			if (UCA0RXBUF == '['  && !flagACKTx) {
				flagACKTx = TRUE;
				flagRxMode = TRUE;

			}

			if (UCA0RXBUF != ']' && flagACKTx && !flagRxMode) {
				uartIn[dataLength++] = UCA0RXBUF;

				if (dataLength > BUFF_SIZE-1) {
					dataLength = 0;
				}
			}

			if (UCA0RXBUF == ']' && flagACKTx) {
				flagACKTx = FALSE;
				flagRxMode = TRUE;
			}

			//Transmitter
			if (UCA0RXBUF == '>' && !flagACKTx) {
				flagACKTx = TRUE;
				flagRxMode = FALSE;
			}

			if (UCA0RXBUF == '>' && flagACKTx) {
				flagACKTx = FALSE;
			}*/

		//}

		if (cnter1 > BUFF_SIZE-1) {
			cnter1 = 0;
		}


		break;
	default: break;
	}
	__low_power_mode_off_on_exit();
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	clkTimer++;
	testCounter++;
	if (clkTimer == 2) {
		timeFlag = TRUE;
		clkTimer = 0;
	}
	__low_power_mode_off_on_exit();
}
