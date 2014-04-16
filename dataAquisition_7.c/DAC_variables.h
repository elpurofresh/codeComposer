#ifndef DAC_VARIABLES_H_
#define DAC_VARIABLES_H_

#define Red_LED		BIT0
#define Green_LED	BIT6
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define ADR_BUS0	BIT3			// address bus line 0 on P1.4
#define ADR_BUS1	BIT0			// address bus line 1 on P2.0
#define ON  		1
#define OFF			0

unsigned char byteRX = 0;

volatile unsigned int PH=0;
volatile unsigned int Ox=0;
volatile unsigned int temp=0;
volatile unsigned int randNumber;
/*volatile unsigned char ON = 1;
volatile unsigned char OFF = 0;*/


#endif /*DAC_VARIABLES_H_*/
