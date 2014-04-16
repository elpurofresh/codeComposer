//******************************************************************************
//  MSP430FR57xx Demo - USCI_B0 I2C Master TX single bytes to MSP430 Slave
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the master code. It continuously
//  transmits 00h, 01h, ..., 0ffh and demonstrates how to implement an I2C
//  master transmitter sending a single byte using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
//
//                                /|\  /|\
//                MSP430FR5739    10k  10k     MSP430FR5739
//                   slave         |    |         master
//             -----------------   |    |   -----------------
//           -|XIN  P1.6/UCB0SDA|<-|----+->|P1.6/UCB0SDA  XIN|-
//            |                 |  |       |                 |
//           -|XOUT             |  |       |             XOUT|-
//            |     P1.7/UCB0SCL|<-+------>|P1.7/UCB0SCL     |
//            |                 |          |                 |
//
//   P. Thanigai
//   Texas Instruments Inc.
//   August 2010
//   Built with CCS V4 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430fr5739.h"

unsigned char TXData = 'A';                    // Pointer to TX data
unsigned char TXByteCtr;

int portFlag = 0;
unsigned char RXData = 'x';

void main(void)

{
	WDTCTL = WDTPW + WDTHOLD;
	/*// Init SMCLK = MCLk = ACLK = 1MHz
	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz
	CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // set ACLK = MCLK = DCO
	CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // set all dividers to 1MHz*/

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
		SFRIFG1 &= ~OFIFG;					// Clear XT1 fault flag
	}while (SFRIFG1&OFIFG);                 // Test oscillator fault flag

	// Configure Pins for I2C
	P1SEL1 |= BIT6 + BIT7;                  // Pin init

	UCB0CTLW0 |= UCSWRST;                   // put eUSCI_B in reset state
	UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;// I2C master mode, SMCLk
	UCB0BRW = 0x0008;                          // baudrate = SMCLK / 12
	UCB0I2CSA = 0x48;                       // address slave is 48hex
	UCB0CTLW0 &=~ UCSWRST;	            //clear reset register
	UCB0IE |= UCTXIE0 + UCNACKIE;           //transmit and NACK interrupt enable

	PJDIR |= BIT0 + BIT1;
	PJOUT &= ~(BIT0 + BIT1);

	P3DIR |= BIT5 + BIT7;
	P3OUT &= ~(BIT5 + BIT7);


	/*P1DIR &= ~BIT4;			// P1.4 set to input
	P1REN |= BIT4;			// P1.4 pull-up set on it
	P1OUT &= ~BIT4;			// P1.4 set to Low
	P1IES &= ~BIT4;			// P1.4 Lo/Hi edge interrupt
	P1IE |= BIT4;			// P1.4 interrupt enabled
	P1IFG &= ~BIT4;			// P1.4 IFG cleared*/

	while(1)
	{
		//if (portFlag == 1) {
			//__delay_cycles(4000000);                 // Delay between transmissions
			PJOUT ^= BIT0;

			TXByteCtr = 1;                          // Load TX byte counter
			while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

			UCB0CTLW0 |= UCTR + UCTXSTT;            // I2C TX, start condition

		//	portFlag = 0;
		//}

		__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts

		// Remain in LPM0 until all data
		// is TX'd
		//TXData++;                               // Increment data byte
	}

}

#pragma vector = USCI_B0_VECTOR 
__interrupt void USCIB0_ISR(void) 
{	
	switch(__even_in_range(UCB0IV,0x1E))
	{
	case 0x00: break;                    // Vector 0: No interrupts break;
	case 0x02: break;

	case 0x04:
		P3OUT |= BIT5;
		UCB0CTLW0 |= UCTXSTT;             // Re-send start if NACK
		__delay_cycles(4000000);
		P3OUT &= ~BIT5;
		break;                            // Vector 4: NACKIFG break;

	case 0x16:
		PJOUT ^= BIT1;
		RXData = UCB0RXBUF;    // Get RX data
		break;

	case 0x18:
		P3OUT |= BIT7;
		if (TXByteCtr)                    	// Check TX byte counter
		{
			UCB0TXBUF = TXData;             // Load TX buffer
			TXByteCtr--;                    // Decrement TX byte counter
		}
		else
		{
			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
			UCB0IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
			__bic_SR_register_on_exit(CPUOFF);// Exit LPM0
		}
		__delay_cycles(4000000);
		P3OUT &= ~BIT7;
		break;                            // Vector 26: TXIFG0 break;

	default: break;
	}
}

/*// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	portFlag = 1;
	//OperationMode = GetTime;
	P1IFG &= ~BIT4;                         // Clear P1.4 IFG
}*/

