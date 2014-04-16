//******************************************************************************
//   MSP430 USCI I2C Transmitter and Receiver
//
//  Description: This code configures the MSP430's USCI module as 
//  I2C master capable of transmitting and receiving bytes. 
//
//  ***THIS IS THE MASTER CODE***
//
//                    Master                   
//                 MSP430F2619             
//             -----------------          
//         /|\|              XIN|-   
//          | |                 |     
//          --|RST          XOUT|-    
//            |                 |        
//            |                 |        
//            |                 |       
//            |         SDA/P3.1|------->
//            |         SCL/P3.2|------->
//
// Note: External pull-ups are needed for SDA & SCL
//
// Uli Kretzschmar
// Texas Instruments Deutschland GmbH
// November 2007
// Built with IAR Embedded Workbench Version: 3.42A
//******************************************************************************
#include "msp430fr5739.h"
#include "i2C_master.h"

signed char byteCtr;
unsigned char *TI_receive_field;
unsigned char *TI_transmit_field;

//------------------------------------------------------------------------------
// void TI_USCI_I2C_receiveinit(unsigned char slave_address, 
//                              unsigned char prescale)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//-----------------------------------------------------------------------------
void receiveI2C_init(unsigned char slave_address)
{
	P3OUT ^= BIT4;
	// Configure pins
	P1SEL1    |= SDA_PIN + SCL_PIN;

	UCB0CTLW0 |= UCSWRST;	            		//Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC;	//I2C mode, Master mode, sync
	UCB0CTLW1 |= UCASTP_2;		    			//automatic stop generated
	//after UCB0TBCNT is reached
	UCB0BRW    = 0x0008; 		    			// baudrate = SMCLK / 8
	UCB0TBCNT |= 0x0001;		    			//number of bytes to be received
	UCB0I2CSA  = slave_address;	    			//slave address
	UCB0CTL1  &=~UCSWRST;
	UCB0IE    |= UCRXIE0 + UCNACKIE; //+UCBCNTIE;
	P3OUT ^= BIT4;
}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_transmitinit(unsigned char slave_address, 
//                               unsigned char prescale)
//
// This function initializes the USCI module for master-transmit operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
void transmitI2C_init(unsigned char slave_address)
{
	PJOUT ^= BIT1;
	// Configure pins
	P1SEL1    |= SDA_PIN + SCL_PIN;

	UCB0CTLW0 |= UCSWRST;	            		//Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC;	//I2C mode, Master mode, sync
	UCB0CTLW1 |= UCASTP_2;		    			//automatic stop generated
												//after UCB0TBCNT is reached
	UCB0BRW    = 0x0008; 		    			// baudrate = SMCLK / 8
	UCB0TBCNT |= 0x0001;		    			//number of bytes to be received
	UCB0I2CSA  = slave_address;	    			//slave address
	UCB0CTL1  &= ~UCSWRST;
	UCB0IE    |= UCTXIE0 + UCNACKIE; //+UCBCNTIE;		// Enable TX ready, No ACK and Counter interrupt

	PJOUT ^= BIT1;
}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_receive(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-receiver mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be read
//       unsigned char *field     =>  array variable used to store received data
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-transmit mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be transmitted
//       unsigned char *field     =>  array variable. Its content will be sent.
//------------------------------------------------------------------------------
void transmitI2C(unsigned char byteCount, unsigned char *field)
{

	TI_transmit_field = field;
	byteCtr = byteCount;
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
	UCB0CTLW0 |= UCTR + UCTXSTT;            // I2C TX, start condition

}

//------------------------------------------------------------------------------
// unsigned char TI_USCI_I2C_notready()
//
// This function is used to check if there is commuincation in progress. 
//
// OUT:  unsigned char  =>  0: I2C bus is idle, 
//                          1: communication is in progress
//------------------------------------------------------------------------------
unsigned char TI_USCI_I2C_notready(){
	PJOUT ^= BIT2;
	//_delay_cycles(1000000);
	return (UCB0STAT & UCBBUSY);
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
{

	switch(__even_in_range(UCB0IV,0x1E))
	{
	case 0x00: break; 						// Vector 0: No interrupts break;
	case 0x02: break; 						// Vector 2: ALIFG
	case 0x04:								// Vector 4: NACKIFG
		UCB0CTLW0 |= UCTXSTT;             // Re-send start if NACK
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
			//__bic_SR_register_on_exit(CPUOFF);// Exit LPM0
		}
		break;


	case 0x1a: break; 							// Vector 28: BCNTIFG break;
	case 0x1c: break; 							// Vector 30: clock low timeout break;
	case 0x1e: break; 							// Vector 32: 9th bit break;
	default: break;

	}
}


