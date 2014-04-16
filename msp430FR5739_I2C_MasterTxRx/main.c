/*
 * main.c
 */

#include "msp430fr5739.h"

#define SDA_PIN BIT6                // msp430FR5739 UCB0SDA pin 1.6
#define SCL_PIN BIT7                // msp430FR5739 UCB0SCL pin 1.7
#define SLAVE_ADDRESS 	0x48

void initClock(void);
void transmitI2C_init(unsigned char slave_address);
void receiveI2C_init(unsigned char slave_address);
void transmitI2C(unsigned char byteCount, unsigned char *field);
void receiveI2C(unsigned char byteCount, unsigned char *field);

void initPorts(void);


signed char byteCtr;
unsigned char *TI_receive_field;
unsigned char *TI_transmit_field;

unsigned char charToSend[1] = "A";


void main(void) {

	initClock();
	initPorts();

	while(1) {

		PJOUT ^= BIT0;             	// Toggle P1.0 using exclusive-OR
		transmitI2C_init(SLAVE_ADDRESS);
		//while (UCB0STAT & UCBBUSY);
		transmitI2C(1, charToSend);
		PJOUT ^= BIT0;             	// Toggle P1.0 using exclusive-OR

		__bis_SR_register_on_exit(LPM3_bits + GIE);

	}
}

void initClock(void)
{
	WDTCTL = WDTPW + WDTHOLD;

	CSCTL0_H = 0xA5;							// Un-lock clock register
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELA_0 + SELS_3 + SELM_3;        // set ACLK = XT1; MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_3 + DIVM_3;        // set all dividers
	CSCTL4 |= XT1DRIVE_0;
	CSCTL4 &= ~XT1OFF;
	CSCTL0_H = 0x01;                          // Lock Clock Register

}

void initPorts(void)
{
	PJDIR |= (BIT0 + BIT1 + BIT2 + BIT3);
	PJOUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);

	P3DIR |= (BIT4 + BIT5 + BIT6 + BIT7);
	P3OUT &= ~(BIT4 + BIT5 + BIT6 + BIT7);
}

void transmitI2C_init(unsigned char slave_address)
{
	PJOUT ^= BIT1;

	P1SEL1    |= SDA_PIN + SCL_PIN;				// Configure pins

	UCB0CTLW0 |= UCSWRST;	            		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC;		// I2C mode, Master mode, sync mode
	UCB0CTLW1 |= UCASTP_2;		    			// automatic stop generated
												// after UCB0TBCNT is reached
	UCB0BRW    = 0x0008; 		    			// baudrate = SMCLK / 8
	UCB0TBCNT |= 0x0001;		    			// number of bytes to be received
	UCB0I2CSA  = slave_address;	    			// slave address
	UCB0CTL1  &= ~UCSWRST;
	UCB0IE    |= UCTXIE0 + UCNACKIE + UCBCNTIE;	// Enable TX ready, No ACK and Counter interrupt

	PJOUT ^= BIT1;
}


void receiveI2C_init(unsigned char slave_address)
{
	P3OUT 	  ^= BIT4;

	P1SEL1    |= SDA_PIN + SCL_PIN;				// Configure pins

	UCB0CTLW0 |= UCSWRST;	            		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC;	// I2C mode, Master mode, sync
	UCB0CTLW1 |= UCASTP_2;		    			// automatic stop generated
												// after UCB0TBCNT is reached
	UCB0BRW    = 0x0008; 		    			// baudrate = SMCLK / 8
	UCB0TBCNT |= 0x0001;		    			// number of bytes to be received
	UCB0I2CSA  = slave_address;	    			// slave address
	UCB0CTL1  &= ~UCSWRST;
	UCB0IE    |= UCRXIE0 + UCNACKIE + UCBCNTIE;

	P3OUT 	  ^= BIT4;
}

void transmitI2C(unsigned char byteCount, unsigned char *field)
{

	TI_transmit_field = field;
	byteCtr = byteCount;
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
	UCB0CTLW0 |= UCTR + UCTXSTT;            // I2C TX, start condition

}

void receiveI2C(unsigned char byteCount, unsigned char *field)
{
	P3OUT ^= BIT5;

	TI_receive_field = field;
	if ( byteCount == 1 ){
		byteCtr = 0 ;
		__disable_interrupt();
		UCB0CTL1 |= UCTXSTT;                      // I2C start condition
		while (UCB0CTL1 & UCTXSTT);               // Start condition sent?
		UCB0CTL1 |= UCTXSTP;                      // I2C stop condition
		__enable_interrupt();
	} else if ( byteCount > 1 ) {
		byteCtr = byteCount - 2 ;
		UCB0CTL1 |= UCTXSTT;                      // I2C start condition
	} else
		while (1);                                // illegal parameter

	P3OUT ^= BIT5;
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
{

	switch(__even_in_range(UCB0IV,0x1E))
	{
	case 0x00: break; 						// Vector 0: No interrupts break;
	case 0x02: break; 						// Vector 2: ALIFG
	case 0x04:								// Vector 4: NACKIFG

		UCB0CTL1 |= UCTXSTT;				// I2C start condition (UCB0CTLW0 = UCB0CTL0(high bits, 15-8) + UCB0CTL1(low bits, 7-0))

		//UCB0CTLW0 |= UCTXSTT;             // Re-send start if NACK
		//UCB0CTLW0 |= UCTXSTP;
		//UCB0IFG &= ~UCNACKIFG;

		break;
	case 0x06: break; 						// Vector 6: STTIFG
	case 0x08:       						// Vector 8: STPIFG
		UCB0IFG &= ~UCSTPIFG;				// Clear stop condition int flag
		break;

	/* Not necessary
	case 0x0a: break; 						// Vector 10: RXIFG3 break;
	case 0x0c: break; 						// Vector 14: TXIFG3 break;
	case 0x0e: break; 						// Vector 16: RXIFG2 break;
	case 0x10: break; 						// Vector 18: TXIFG2 break;
	case 0x12: break; 						// Vector 20: RXIFG1 break;
	case 0x14: break; 						// Vector 22: TXIFG1 break;*/

	case 0x16:								// Vector 24: RXIFG0 break;

		if ( byteCtr == 0 ){
			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
			*TI_receive_field = UCB0RXBUF;	// Put last byte into memory buffer
			TI_receive_field++;
		}
		else {
			*TI_receive_field = UCB0RXBUF;
			TI_receive_field++;
			byteCtr--;
		}
		__bic_SR_register_on_exit(LPM3_bits + GIE);

		break;

	case 0x18:								// Vector 26: TXIFG0
		if (byteCtr == 0)                   // Check TX byte counter
		{
			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
			UCB0IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
		}
		else
		{
			UCB0TXBUF = *TI_transmit_field;
			TI_transmit_field++;            // Load TX buffer
			byteCtr--;                    	// Decrement TX byte counter
			__bic_SR_register_on_exit(LPM3_bits + GIE);// Exit LPM0
		}
		break;


	case 0x1a:

		PJOUT ^= BIT1;

		break; 								// Vector 28: BCNTIFG break;
	case 0x1c: break; 						// Vector 30: clock low timeout break;
	case 0x1e: break; 						// Vector 32: 9th bit break;
	default: break;

	}
}







