/*
 * main.c
 */

#include <msp430.h>
#include <msp430g2253.h>
#include <msp430x16x.h>
#include "hardware_board.h"


char tmp;
unsigned char status = 1;

void main(void) {

	WDTCTL = WDTPW + WDTHOLD; // disable watch dog for testing
	UCA0CTL1 |=  UCSWRST; // **Put state machine in reset**
	UCA0CTL1 |= UCSSEL_2; // select SMCLK
	UCA0CTL0 =UCSYNC|  UCMST| UCMSB| UCCKPL;//  ;

	// SET TO 400 KHZ
	UCA0BR0 = 0x28;
	UCA0BR1 = 0x0;
	UCA0CTL1 &=  ~UCSWRST;   // **Initialize USCI state machine**

	//Initialization of the SD-card
	/*while (status != 0)                       // if return in not NULL an error did occur and the
		// SD-card will be initialized again
	{
		status = mmcInit();
		timeout++;
		if (timeout == 150)                      // Try 50 times till error
		{
			break;
		}
	}

	while ((mmcPing() != MMC_SUCCESS)); */     // Wait till card is inserted

	while (1)	{
		spi_cs_assert();
		//__delay_cycles(250000);
		spi_send_byte((0x40));
		spi_send_byte((0x00));
		spi_send_byte((0x00));
		spi_send_byte((0x00));
		spi_send_byte((0x00));
		spi_send_byte((0x95));

		do	{
			tmp = spi_rcv_byte();
			i++;
		}

		while (((tmp & 0x80) != 0) && i < 100);


		spi_cs_deassert();
		__delay_cycles(250000);
	}
}

/*char mmcInit(void) {
	//Raise CS and MOSI for 80 clock cycles
	//SendByte(0xff) 10 times with CS high
	//RAISE CS
	int i;

	// Port x Function           Dir       On/Off
	//         mmcCS         Out       0 - Active 1 - none Active
	//         Dout          Out       0 - off    1 - On -> init in SPI_Init
	//         Din           Inp       0 - off    1 - On -> init in SPI_Init
	//         Clk           Out       -                 -> init in SPI_Init
	//         mmcCD         In        0 - card inserted

	// Init Port for MMC (default high)
	SPI_PxOUT |= SPI_SIMO + SPI_UCLK;
	SPI_PxDIR |= SPI_SIMO + SPI_UCLK;


	// Chip Select
	MMC_CS_PxOUT |= MMC_CS;
	MMC_CS_PxDIR |= MMC_CS;

	// Card Detect
	MMC_CD_PxDIR &=  ~MMC_CD;

	// Init SPI Module
	UCA0CTL0 = UCMST+UCCKPL+UCMSB+UCSYNC;     // 3-pin, 8-bit SPI master
	UCA0CTL1 = UCSSEL_2 + UCSWRST;            // SMCLK
	UCA0BR0 |= 0x02;                          // UCLK/2
	UCA0BR1 = 0;
	UCA0MCTL = 0;
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	//initialization sequence on PowerUp
	CS_HIGH();
	for(i=0;i<=9;i++)
		spiSendByte(DUMMY_CHAR);

	return (mmcGoIdle());
}

// set MMC in Idle mode
char mmcGoIdle()
{
	char response=0x01;
	CS_LOW();

	//Send Command 0 to put MMC in SPI mode
	mmcSendCmd(MMC_GO_IDLE_STATE,0,0x95);
	//Now wait for READY RESPONSE
	if(mmcGetResponse()!=0x01)
		return MMC_INIT_ERROR;

	while(response==0x01)
	{
		CS_HIGH();
		spiSendByte(DUMMY_CHAR);
		CS_LOW();
		mmcSendCmd(MMC_SEND_OP_COND,0x00,0xff);
		response=mmcGetResponse();
	}
	CS_HIGH();
	spiSendByte(DUMMY_CHAR);
	return (MMC_SUCCESS);
}*/

/* Send a single byte over the SPI port */
void spi_send_byte(unsigned char input) {
	//IFG1 &= ~URXIFG0;
	/* Send the byte */
	while ((UCB1STAT & UCBUSY)); // SPI ready?
	while (!(UCB1IFG&UCTXIFG));           // USCI_B1 TX buffer ready?
	UCB1TXBUF = input;
	/* Wait for the byte to be sent */
	//UCTXIFG is set when UCxxTXBUF empty.
	while ((UCB1IFG & UCTXIFG) == 0) { }
}

//Send one byte via SPI
unsigned char spiSendByte(const unsigned char data)
{
  while (halSPITXREADY ==0);    // wait while not ready for TX
  halSPI_SEND(data);            // write
  while (halSPIRXREADY ==0);    // wait for RX buffer (full)
  return (halSPIRXBUF);
}

/* Receive a byte. Output an 0xFF (the bus idles high) to receive the byte */
unsigned char spi_rcv_byte() {

	unsigned char tmp;
	/* Send the byte */
	tmp = UCB1RXBUF; // clear buffer
	while ((UCB1STAT & UCBUSY)); // SPI ready?
	spi_send_byte(0xFF);
	/* Wait for the byte to be received */
	while ((UCB1IFG & UCRXIFG) == 0) { }
	while ((UCB1STAT & UCBUSY)); // SPI ready?
	tmp = UCB1RXBUF;
	return (tmp);
}
