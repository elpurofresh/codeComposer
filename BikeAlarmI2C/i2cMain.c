//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Master TX/RX multiple bytes from MSP430 Slave
//                     with a repeated start in between TX and RX operations.
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave, then a repeated start is generated followed by a
//  receive operation. This is the master code. This code demonstrates how to
//  implement an I2C repeated start with the USCI module using the USCI_B0 TX
//  interrupt.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.2MHz
//
//	***to be used with msp430x22x4_uscib0_i2c_13.c***
//
//                                /|\  /|\
//               MSP430F24x      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P3.1/UCB0SDA|<-|---+->|P3.1/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P3.2/UCB0SCL|<-+----->|P3.2/UCB0SCL     |
//            |                 |         |                 |
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include "msp430g2553.h"
#include "TI_USCI_I2C_master.h"
#include <stdbool.h>

#define NUM_BYTES_TX 1                         // How many bytes?
#define NUM_BYTES_RX 2
#define READ		 1
#define WRITE		 0

#define Error       BIT0

int RXByteCtr, RPT_Flag = 0;                // enables repeated start when 1
volatile unsigned char RxBuffer[10];       // Allocate 128 byte of RAM
unsigned char *PTxData;                     // Pointer to TX data
unsigned char *PRxData;                     // Pointer to RX data
unsigned char TXByteCtr, RX = 0;
unsigned char MSData = 0x00;
unsigned char TxData[1] = {0x0D};			// Register addresses to be accessed on the accelometer
volatile int readWrite = 0;					// Read = 0, Write = 1

char TestData[1] = {0x0D};

volatile int read = 0;
volatile int write = 0;

unsigned char tx_byte_count 	= 0;
unsigned char rx_byte_count 	= 0;
unsigned char tx_byte_counter	= 0;
unsigned char rx_byte_counter	= 0;
unsigned char tx_rx 			= 0;

void Setup_I2C(void);
void Setup_TX(void);
void Setup_RX(void);

void SingleByteRead(void);

void Transmit(unsigned char * data, const int numberOfBytes);
void Receive(const int numberOfBytes);

static char *i2a(unsigned i, char *a, unsigned r);
char *itoa(int i, char *a, int r);
void TX(char *tx_message);
void SetupUART(void);

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0 P1.6 -> SDA
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0 P1.7 -> SCL

	P1DIR |= BIT0; //(BIT0 + BIT4);			// Set this pin as outputs
	P1OUT &= 0;								// Reset their values to 0

	int i = 0;
	for (i = 0; i < 128; ++i) {
		RxBuffer[i] = '#';
	}

	//P1OUT |= BIT4;

	Setup_I2C();
	SetupUART();

	//while(1){

	/*while ( TI_USCI_I2C_notready() )         // wait for bus to be free
		{
			P1OUT ^= BIT0;
			_delay_cycles(500000);
		}*/

	tx_rx = 0; //transmit
	tx_byte_count   = 1  + 1;
	tx_byte_counter  = 1;
	UCB0CTL1 |= UCTR + UCTXSTT; 				  // this immediately sets UCTXIFG

	//while(UCB0CTL1 & UCTXSTT);				  // wait for end of start condition
	TX("OUT: ");
	TX(TestData);
	TX("!\r\n");

	/*if (IFG2 & UCNACKIFG) {
			UCB0CTL1 |= UCTXSTP;
		} // slave didn't respond*/

	//__bis_SR_register(CPUOFF + GIE);

	/*tx_rx = 1;
	rx_byte_count = 1 + 1;
	rx_byte_counter = 1;
	UCB0CTL1 &= ~UCTR;
	UCB0CTL1 |= UCTXSTT;
	__bis_SR_register(CPUOFF + GIE);*/

	//}
}

void Setup_I2C(void)
{
	_DINT();
	IE2 &= ~(UCB0RXIE + UCB0TXIE);
	//while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
	UCB0CTL1 |= UCSWRST;                      	// Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     	// I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            	// Use SMCLK, keep SW reset
	UCB0BR0 = 12;                             	// fSCL = SMCLK/12 = ~100kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x1D;                     	  	// Slave Address 0x1D, read/write controlled by ~UCTR/UCTR bit
	UCB0CTL1 &= ~UCSWRST;                     	// Clear SW reset, resume operation
	IE2 |= UCB0TXIE + UCB0RXIE;					// Enable TX and RX interrupts
	IE2 |= UCNACKIE;
	//_delay_cycles(20000);						// Small start delay
}

void Setup_TX(void)
{
	_DINT();
	RX = 0;
	IE2 &= ~UCB0RXIE;
	while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent// Disable RX interrupt
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x1D;                     	  // Slave Address 0x1D, read/write controlled by ~UCTR/UCTR bit
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0TXIE;                          // Enable TX interrupt
}

void Setup_RX(void)
{
	_DINT();
	RX = 1;
	IE2 &= ~UCB0TXIE;
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x1D;                     	  // Slave Address 0x1D, read/write controlled by ~UCTR/UCTR bit
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0RXIE;                          // Enable RX interrupt
}

void SingleByteRead(void)
{
	UCB0CTL1 |= UCTR|UCTXSTT; 				  // this immediately sets UCTXIFG
	UCB0TXBUF = TxData[0];
	while(UCB0CTL1 & UCTXSTT);				  // wait for end of start condition

	if (IFG2 & UCNACKIFG) {
		UCB0CTL1 |= UCTXSTP;
		P1OUT |= BIT0;
		return;
	} // slave didn't respond

	while(!(IFG2 & UCB0TXIFG)); 				// wait until byte starts to send

	UCB0CTL1 &= ~UCTR;					 	// Change to Receive
	UCB0CTL1 |= UCTXSTT;					// Send Restart condition
	while(!(IFG2 & UCB0RXIFG))
		RxBuffer[0] = UCB0RXBUF;

	UCB0CTL1 |= UCTXNACK + UCTXSTP;

}

void Transmit(unsigned char * data, const int numberOfBytes)
{
	PTxData = data; //&MSData;              // TX array start address
	TXByteCtr = numberOfBytes; //NUM_BYTES_TX;               // Load TX byte counter
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}

void Receive(const int numberOfBytes)
{
	PRxData = (unsigned char *)RxBuffer;    // Start of RX buffer
	RXByteCtr = numberOfBytes; //NUM_BYTES_RX-1;              // Load RX byte counter
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 |= UCTXSTT;                    // I2C start condition
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}

static char *i2a(unsigned i, char *a, unsigned r)
{
	if (i/r > 0) a = i2a(i/r,a,r);
	*a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
	return a+1;
}

char *itoa(int i, char *a, int r)
{
	if ((r < 2) || (r > 36)) r = 10;
	if (i < 0)
	{
		*a = '-';
		*i2a(-(unsigned)i,a+1,r) = 0;
	}
	else *i2a(i,a,r) = 0;
	return a;
}

void TX(char *tx_message)
{
	unsigned int i=0; //Define end of string loop int
	char *message; // message variable
	unsigned int message_num; // define ascii int version variable
	message = tx_message; // move tx_message into message
	while(1)
	{
		if(message[i]==0) // If end of input string is reached, break loop.
		{break;}
		message_num = (int)message[i]; //Cast string char into a int variable
		UCA0TXBUF = message_num; // write INT to TX buffer
		i++; // increase string index
		__delay_cycles(10000); //transmission delay
		if(i>50) //prevent infinite transmit
		{
			P1OUT |= Error;
			break;
		}
	} // End TX Main While Loop
} // End TX Function

void SetupUART(void)
{
	P1SEL = BIT1 + BIT2;            // Set P1.1 to RXD and P1.2 to TXD
	P1SEL2 = BIT1 + BIT2;            //
	UCA0CTL1 |= UCSSEL_2;            // Have USCI use SMCLK AKA 1MHz main CLK
	UCA0BR0 = 104;                  // Baud: 9600, N= CLK/Baud, N= 10^6 / 9600
	UCA0BR1 = 0;                  // Set upper half of baud select to 0
	UCA0MCTL = UCBRS_1;               // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;             // Start USCI

}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	//RxBuffer[read++] = UCB0RXBUF;

	if (UCB0STAT & UCNACKIFG){            // send STOP if slave sends NACK
		UCB0CTL1 |= UCTXSTP;
		UCB0STAT &= ~UCNACKIFG;
	}
	//if (read >= 128) {
	//	read = 0;
	//}
}


#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{

	if(tx_rx == 0)
	{
		if (tx_byte_counter > 0)					//Check TX byte counter
		{
			//UCB0TXBUF = TxData[tx_byte_count - tx_byte_counter];			// Load TX buffer
			UCB0TXBUF = TxData[0];			// Load TX buffer
			tx_byte_counter--;						//Decrement TX byte counter
		}
		else if(tx_byte_counter == 0)
		{
			UCB0CTL1 |= UCTXSTP;					//I2C stop condition
			while (UCB0CTL1 & UCTXSTP);				//Ensure stop condition got sent
			IFG2 &= ~UCB0TXIFG;						//Clear USCI_B0 TX int flag
			write++;
			__bic_SR_register_on_exit(CPUOFF);		//Exit LPM0
		}
	}
	else if(tx_rx == 1)
	{
		if (rx_byte_counter > 0)					//Check RX byte counter
		{
			RxBuffer[rx_byte_count - rx_byte_counter] = UCB0RXBUF;
			rx_byte_counter--;						//Decrement RX byte counter
		}
		else if(rx_byte_counter == 0)
		{
			UCB0CTL1 |= UCTXNACK + UCTXSTP;
			//UCB0CTL1 |= UCTXSTP;					// I2C stop condition
			while (UCB0CTL1 & UCTXSTP);				// Ensure stop condition got sent
			//RxBuffer[rx_byte_count - (rx_byte_counter + 1)] = UCB0RXBUF;
			//RxBuffer[rx_byte_count - (rx_byte_counter + 1)] = UCB0RXBUF;
			//IFG2 &= ~UCB0RXIFG;						// Clear USCI_B0 RX int flag
			read++;
			__bic_SR_register_on_exit(CPUOFF);		// Exit LPM0
		}
	}





	/*if (IFG2 & UCB0RXIFG){							// Receiving Mode
		if ( RXByteCtr == 0 ){
			UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	 *TI_receive_field = UCB0RXBUF;
			TI_receive_field++;
		}
		else {
	 *TI_receive_field = UCB0RXBUF;
			TI_receive_field++;
			RXByteCtr--;
		}
	}
	else {											// Transmitting Mode
		if (TXByteCtr == 0){
			UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
			IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
		}
		else {
			UCB0TXBUF = *TI_transmit_field;
			TI_transmit_field++;
			TXByteCtr--;
		}
	}*/

	//UCB0TXBUF = TxData[0];
}
