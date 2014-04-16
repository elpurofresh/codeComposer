/*** USCI master library ************************************************************

In this file the usage of the USCI I2C master library without DMA support is 
shown. This library uses pointers to specify what data is to be sent. 

When calling the TI_USCI_I2C_receive or TI_USCI_I2C_transmit routines 
the number of bytes, which are to be transmitted or received have to be passed as 
well as a pointer to a data field, that contains(or stores) the data.

This code checks if there is a slave with address 0x50 is connected to the I2C
bus and if the slave device is present, bytes are received and transmitted.

Uli Kretzschmar
MSP430 Systems
Freising
 *******************************************************************************/
//#include "msp430x26x.h"
#include "msp430g2553.h"
#include "TI_USCI_I2C_master.h"

unsigned char timercounter;
unsigned char array[40] = { 0x0, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb };
unsigned char store[40] = { 13, 13, 13, 13, 13, 13, 13, 0, 0, 0};


void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	/*BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;*/
	P1DIR |= 0xFF; //(BIT0 + BIT1 + BIT2 + BIT5);
	P1OUT &= 0; //~(BIT0 + BIT1 + BIT2 + BIT5);

	_EINT();

	TI_USCI_I2C_transmitinit(0x1D,0x12);  // init transmitting with USCI

	while ( TI_USCI_I2C_notready() );         // wait for bus to be free

	if ( TI_USCI_I2C_slave_present(0x1D) )    // slave address may differ from
	{                                         // initialization
		P1OUT |= BIT0;							//Red LED P1.0
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free
		TI_USCI_I2C_transmit(1,array);       // start transmitting
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free
		TI_USCI_I2C_transmit(1,array);       // start transmitting



		TI_USCI_I2C_receiveinit(0x1D,0x12);   // init receiving with USCI
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free
		TI_USCI_I2C_receive(4,store);
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free

		TI_USCI_I2C_transmitinit(0x1D,0x12);  // init transmitting with
	}

	/*TI_USCI_I2C_transmitinit(0x50,0x12);  // init transmitting with USCI
	while ( TI_USCI_I2C_notready() );         // wait for bus to be free
	if ( TI_USCI_I2C_slave_present(0x50) )    // slave address may differ from
	{                                         // initialization
		TI_USCI_I2C_receiveinit(0x50,0x12);   // init receiving with USCI
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free
		TI_USCI_I2C_receive(4,store);
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free

		TI_USCI_I2C_transmitinit(0x50,0x12);  // init transmitting with
		while ( TI_USCI_I2C_notready() );         // wait for bus to be free
		TI_USCI_I2C_transmit(4,array);       // start transmitting
	}*/

	LPM3;

}

