//slave
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
#include  "RTC_variables.h"
#include  "RTC_functions.h"

void processTime(int);

int sec = 0;
int slot = 0;
int sec_0 = 0;
int sec_1 = 0;
int sec_2 = 0;

char *testSend;

unsigned char byteRX;

unsigned int writeProcess = 9;

void main(void)
{
	configClocks();
	setupUART();
	setupOutputs();
	setupTimer();

	_EINT();

	while(1)
	{

		byteRX = '#';
		if (byteRX == 'C') {
			turnTXD(ON);
			blinkLEDRed(1);
			UART_putChar('K');
			blinkLEDGreen(2);
			turnTXD(OFF);
		}

		if (byteRX == 'Q') {
			processTime(sec);
			turnTXD(ON);
			//sleepHalf();
			UART_putChar('J');
			/*//sleepHalf();
			UART_putChar(slot);
			//sleepHalf();
			UART_putInt(sec_0);
			//sleepHalf();
			UART_putChar(sec_1);
			//sleepHalf();
			UART_putChar(sec_2);
			blinkLEDRednGreen(1);*/
			turnTXD(OFF);
		}
	}
}

void processTime(int s){
	if (s > 255) {
		sec_0 = 255;
		sec_1 = sec - 255;
		if (sec_1 > 255) {
			sec_1 = 255;
			sec_2 = sec - 510;
		}
		else{
			sec_2 = 0;
		}
	}
	else{
		sec_0 = s;
		sec_1 = 0;
		sec_2 = 0;
	}
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     // Store RXed character
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
	sec++;
	if (sec > 720) {
		slot++;
		sec = 0;
	}
}

