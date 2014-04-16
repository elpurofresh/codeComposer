/*
 * main.c
 */

#include "msp430g2553.h"
#include <intrinsics.h>
#include <stdint.h>


#define SDA_PIN 		BIT6				// msp430g2553 UCB0SDA pin
#define SCL_PIN 		BIT7				// msp430g2553 UCB0SCL pin
#define N_Message 		2
#define SLAVE_ADDRESS	0x1D

uint8_t RXCount;
uint8_t RXData[N_Message];

void i2cInitialization(void);


enum {
	I2CIdle,
	I2CSucces,
	I2CNack
}	I2CStatus = I2CIdle;


void main(void) {
	
	WDTCTL = WDTPW + WDTHOLD;
	i2cInitialization();
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	_delay_cycles(1000000);
	RXCount = N_Message;
	UCB0CTL1 |= UCTXSTT;

	while (1)
	{
		__low_power_mode_3();

		switch (I2CStatus) {
			case I2CIdle:
				//should never happen
				break;
			case I2CSucces:
				break;
			case I2CNack:
				P1OUT |= BIT0;
				_delay_cycles(1000000);
				P1OUT &= ~BIT0;
				break;
			default:
				I2CStatus = I2CIdle;
				break;
		}
	}
}

void i2cInitialization(void)
{
	P3SEL |= SDA_PIN + SCL_PIN;                 // Assign I2C pins to USCI_B0
	UCB0CTL1 = UCSWRST;                         // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;              // Use SMCLK, keep SW reset
	UCB0BR0 = 12;                         		// Set prescaler
	UCB0BR1 = 0;								// Ignore gral call; own address = 0
	UCB0I2CSA = SLAVE_ADDRESS;                  // Set slave address
	UCB0I2COA = 0;
	UCB0I2CIE = UCNACKIE;
	UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;                            // Enable TX ready interrupt
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	if (RXCount > 0 ) {
		RXData[--RXCount] = UCB0RXBUF;
		if (RXCount == 1) {
			UCB0CTL1 |= UCTXSTP;
		} else if (RXCount == 0) {
			I2CStatus = I2CSucces;
			__low_power_mode_off_on_exit();
		}
	} else {
		IFG2 |= UCB0RXIFG;
	}
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	UCB0STAT &= ~UCNACKIFG;
	UCB0CTL1 |= UCTXSTP;
	I2CStatus = I2CNack;
	__low_power_mode_off_on_exit();
}


