#include "msp430g2553.h"
#include "DAC_variables.h"
#include "DAC_functions.h" 

char port = 0;

void main(void)
{
	configClocks();
	setupOutputs();
	//setupInterrupts();
	//turnTXD(ON);
	setupUART();

	_EINT();

	while(1){
		//if (port) {

		//turnTXD(ON);

		//P1IFG &= ~ADR_BUS0;
		//port = 0;
		//blinkLEDGreen(1);
		//blinkLEDRed(1);

		//int i = 0, timeOut = 10000;

		//while(i < timeOut){

		while (byteRX == 'D') {
			turnTXD(ON);
			blinkLEDRed(1);
			UART_putChar('O');
			blinkLEDGreen(2);
			turnTXD(OFF);
			//byteRX = '#';  //this is to flush the RX buffer
			//i = timeOut;
		}

		//blinkLEDRed(1);
		//i++;
	}
	//}
}


//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     	// Store RXed character
}

/*
#pragma vector=PORT1_VECTOR
__interrupt void port_1(void){
	port = 1;
	blinkLEDRed(1);
	P1IFG &= ~ADR_BUS0;
}
 */
