/*
 * main.c
 */
#include "msp430g2553.h"
#include "TI_USCI_I2C_master.h"
unsigned char array[5] = { 0x0D, 0x1D, 0x1D, 0x4, 0x5 };
unsigned char rxArray[2] = {'$', '$'};
void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Disable Watchdog
	BCSCTL1 = CALBC1_1MHZ;          // Set range for calibrated 1.1 MHz (Basic Timer = DCO)
	DCOCTL = CALDCO_1MHZ;

	P1DIR |= BIT0 + BIT5; //(BIT0 + BIT4);			// Set this pin as outputs
	P1OUT &= ~BIT0;									// Reset their values to 0
	P1OUT |= BIT5;

	_EINT(); // enable interrupts

	//while (1){

	TI_USCI_I2C_transmitinit(0x48,12);
	while(TI_USCI_I2C_notready());
	TI_USCI_I2C_transmit(1,array);

	//while(TI_USCI_I2C_notready());

	//TI_USCI_I2C_receiveinit(0x48, 12);
	//TI_USCI_I2C_receive(1,rxArray);

	/*P1OUT |= BIT0;
		__delay_cycles(1000000);
		P1OUT &= ~BIT0;*/
	//P1OUT &= ~BIT5;
	//}

	/*TI_USCI_I2C_transmitinit(0x48,12);

	while(TI_USCI_I2C_notready());
	TI_USCI_I2C_transmit(1,array);*/

	/*TI_USCI_I2C_receiveinit(0x1D,0x10);
	while(TI_USCI_I2C_notready());
	TI_USCI_I2C_receive(1,rxArray);*/

	/*if (TI_USCI_I2C_slave_present(0x1D)) {
		P1OUT |= BIT0;
		__delay_cycles(1000000);
		P1OUT &= ~BIT0;
		TI_USCI_I2C_transmitinit(0x1D, 0x10);
		while(TI_USCI_I2C_notready());
		TI_USCI_I2C_transmit(3,array);
	}*/

	/*P1OUT |= BIT0;
	__delay_cycles(1000000);
	P1OUT &= ~BIT0;
	P1OUT &= ~BIT5;*/


	/*if (TI_USCI_I2C_slave_present(0x1D)){ 		// check for slave

		P1OUT ^= BIT0;
		_delay_cycles(500000);
		P1OUT ^= BIT0;
		//while(1); 								// trap cpu if slave with
													// address 0x1D doesn’t answer
	}*/

	//while ( TI_USCI_I2C_notready() ); 			// wait for bus to be free
	//TI_USCI_I2C_transmit(1,array); 				// transmit the first 3 bytes

	//LPM0; 									// put CPU to sleep during communication
}
