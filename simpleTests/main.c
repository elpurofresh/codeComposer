/*
 * main.c
 *
 *  Created on: Mar 26, 2012
 *      Author: andres
 */

#include <msp430.h>
#include <msp430x16x.h>
#include "hardware_board.h"

//#define withDMA
unsigned char status = 1;
unsigned int timeout = 0;
unsigned char buffer[256]; //bufferSize = 512

char mmcInit(void);
char mmcGoIdle();
void halSPISetup(void);
unsigned char spiSendByte(const unsigned char data);
void mmcSendCmd (const char cmd, unsigned long data, const char crc);
unsigned char spiReadFrame(unsigned char* pBuffer, unsigned int size);
char mmcGetResponse(void);
char mmcPing(void);
void readSDCard();
void writeSDCard();

int main( void ){

	WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer

	//Initialization of the MMC/SD-card
	while (status != 0)                       // if return in not NULL an error did occur and the
	{											// MMC/SD-card will be initialized again
		status = mmcInit();
		timeout++;
		if (timeout == 150)                      // Try 50 times till error
		{
			break;
		}
	}

	while (mmcPing() != MMC_SUCCESS);      // Wait till card is inserted

	readSDCard();
	writeSDCard();

	mmcGoIdle();                              // set MMC in Idle mode

	while (1);

}

char mmcInit(void) {
	//Raise CS and MOSI for 80 clock cycles
	//SendByte(0xff) 10 times with CS high
	//RAISE CS
	int i;

	// Port x Function       Dir       On/Off
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
	//halSPISetup();
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

/*void halSPISetup(void)
{
  UCA0CTL0 = UCMST+UCCKPL+UCMSB+UCSYNC;     // 3-pin, 8-bit SPI master
  UCA0CTL1 = UCSSEL_2 + UCSWRST;            // SMCLK
  UCA0BR0 |= 0x02;                          // UCLK/2
  UCA0BR1 = 0;
  UCA0MCTL = 0;
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}*/

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
}


//Send one byte via SPI
unsigned char spiSendByte(const unsigned char data)
{
	while (halSPITXREADY ==0);    // wait while not ready for TX
	halSPI_SEND(data);            // write
	while (halSPIRXREADY ==0);    // wait for RX buffer (full)
	return (halSPIRXBUF);
}

// send command to MMC
void mmcSendCmd (const char cmd, unsigned long data, const char crc)
{
	unsigned char frame[6];
	char temp;
	int i;
	frame[0]=(cmd|0x40);
	for(i=3;i>=0;i--){
		temp=(char)(data>>(8*i));
		frame[4-i]=(temp);
	}
	frame[5]=(crc);
	spiSendFrame(frame,6);
}

//Read a frame of bytes via SPI
unsigned char spiReadFrame(unsigned char* pBuffer, unsigned int size)
{
#ifndef withDMA
	unsigned long i = 0;
	// clock the actual data transfer and receive the bytes; spi_read automatically finds the Data Block
	for (i = 0; i < size; i++){
		while (halSPITXREADY ==0);   // wait while not ready for TX
		halSPI_SEND(DUMMY_CHAR);     // dummy write
		while (halSPIRXREADY ==0);   // wait for RX buffer (full)
		pBuffer[i] = halSPIRXBUF;
	}
#else
	U1IFG &= ~(URXIFG1 + URXIFG1);       clear flags
	Get the block
	DMA trigger is UART1 receive for both DMA0 and DMA1
	DMACTL0 &= ~(DMA0TSEL_15 | DMA1TSEL_15);
	DMACTL0 |= (DMA0TSEL_9 | DMA1TSEL_9);
	Source DMA address: receive register.
	DMA0SA = U1RXBUF_;
	Destination DMA address: the user data buffer.
	DMA0DA = (unsigned short)pBuffer;
	The size of the block to be transferred
	DMA0SZ = size;
	Configure the DMA transfer
	DMA0CTL =
			DMAIE   |                          Enable interrupt
			DMADT_0 |                          Single transfer mode
			DMASBDB |                          Byte mode
			DMAEN |                            Enable DMA
			DMADSTINCR1 | DMADSTINCR0;         Increment the destination address

			/*We depend on the DMA priorities here.  Both triggers occur at
           the same time, since the source is identical.  DMA0 is handled
           first, and retrieves the byte.  DMA1 is triggered next, and
           sends the next byte.
         Source DMA address: constant 0xFF (don't increment)*/
			DMA1SA = U1TXBUF_;
			Destination DMA address: the transmit buffer.
			DMA1DA = U1TXBUF_;
			Increment the destination address
			The size of the block to be transferred
			DMA1SZ = count-1;
			Configure the DMA transfer
			DMA1CTL =
					DMADT_0 |                          Single transfer mode
					DMASBDB |                          Byte mode
					DMAEN;                             Enable DMA

					Kick off the transfer by sending the first byte
					halMMC_SEND(0xFF);
					_EINT(); LPM0;  // wait till done
#endif
					return(0);
}

//Send a frame of bytes via SPI
unsigned char spiSendFrame(unsigned char* pBuffer, unsigned int size)
{
#ifndef withDMA
	unsigned long i = 0;
	// clock the actual data transfer and receive the bytes; spi_read automatically finds the Data Block
	for (i = 0; i < size; i++){
		while (halSPITXREADY ==0);   // wait while not ready for TX
		halSPI_SEND(pBuffer[i]);     // write
		while (halSPIRXREADY ==0);   // wait for RX buffer (full)
		pBuffer[i] = halSPIRXBUF;
	}
#else
	/* Get the block */
	/* DMA trigger is UART send */
	DMACTL0 &= ~(DMA0TSEL_15);
	DMACTL0 |= (DMA0TSEL_9);
	/* Source DMA address: the data buffer.  */
	DMA0SA = (unsigned short)pBuffer;
	/* Destination DMA address: the UART send register. */
	DMA0DA = U1TXBUF_;
	/* The size of the block to be transferred */
	DMA0SZ = count;
	/* Configure the DMA transfer*/
	DMA0CTL =
			DMAREQ  |                           /* start transfer */
			DMADT_0 |                           /* Single transfer mode */
			DMASBDB |                           /* Byte mode */
			DMAEN |                             /* Enable DMA */
			DMASRCINCR1 | DMASRCINCR0;          /* Increment the source address */
#endif
	return(0);
}


// mmc Get Response
char mmcGetResponse(void)
{
	//Response comes 1-8bytes after command
	//the first bit will be a 0
	//followed by an error code
	//data will be 0xff until response
	int i=0;

	char response;

	while(i<=64)
	{
		response=spiSendByte(DUMMY_CHAR);
		if(response==0x00)break;
		if(response==0x01)break;
		i++;
	}
	return response;
}


char mmcPing(void)
{
	if (!(MMC_CD_PxIN & MMC_CD))
		return (MMC_SUCCESS);
	else
		return (MMC_INIT_ERROR);
}

void readSDCard(){
	int i = 0;

	// Clear Sectors on MMC
	for (i = 0; i < 512; i++) buffer[i] = 0;
	mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	for (i = 0; i < 512; i++) buffer[i] = 0;
	mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
	for (i = 0; i < 512; i++) {
		if(buffer[i] != 0) {
			P1OUT |= 0x01;
		}
	}

	mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
	for (i = 0; i < 512; i++) {
		if(buffer[i] != 0) {
			P1OUT |= 0x02;
		}
	}
}

void writeSDCard(){
	int i = 0;
	// Write Data to MMC
	for (i = 0; i < 512; i++) buffer[i] = i;
	mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	for (i = 0; i < 512; i++) buffer[i] = i+64;
	mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
	for (i = 0; i < 512; i++) {
		if(buffer[i] != (unsigned char)i) {
			P1OUT |= 0x04;
		}
	}

	mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
	for (i = 0; i < 512; i++) {
		if(buffer[i] != (unsigned char)(i+64)) {
			P1OUT |= 0x08;
		}
	}

	for (i = 0; i < 512; i++)
		mmcReadSector(i, buffer);               // read a size Byte big block beginning at the address.

}

// read a size Byte big block beginning at the address.
char mmcReadBlock(
		const unsigned long address,
		const unsigned long count,
		unsigned char *pBuffer)
{
  char rvalue = MMC_RESPONSE_ERROR;

  // Set the block length to read
  if (mmcSetBlockLength (count) == MMC_SUCCESS)   // block length could be set
  {
    // CS = LOW (on)
    CS_LOW ();
    // send read command MMC_READ_SINGLE_BLOCK=CMD17
    mmcSendCmd (MMC_READ_SINGLE_BLOCK,address, 0xFF);
    // Send 8 Clock pulses of delay, check if the MMC acknowledged the read block command
    // it will do this by sending an affirmative response
    // in the R1 format (0x00 is no errors)
    if (mmcGetResponse() == 0x00)
    {
      // now look for the data token to signify the start of
      // the data
      if (mmcGetXXResponse(MMC_START_DATA_BLOCK_TOKEN) == MMC_START_DATA_BLOCK_TOKEN)
      {
        // clock the actual data transfer and receive the bytes; spi_read automatically finds the Data Block
        spiReadFrame(pBuffer, count);
        // get CRC bytes (not really needed by us, but required by MMC)
        spiSendByte(DUMMY_CHAR);
        spiSendByte(DUMMY_CHAR);
        rvalue = MMC_SUCCESS;
      }
      else
      {
        // the data token was never received
        rvalue = MMC_DATA_TOKEN_ERROR;      // 3
      }
    }
    else
    {
      // the MMC never acknowledge the read command
      rvalue = MMC_RESPONSE_ERROR;          // 2
    }
  }
  else
  {
    rvalue = MMC_BLOCK_SET_ERROR;           // 1
  }
  CS_HIGH ();
  spiSendByte(DUMMY_CHAR);
  return rvalue;
}// mmc_read_block



//char mmcWriteBlock (const unsigned long address)
char mmcWriteBlock (const unsigned long address, const unsigned long count, unsigned char *pBuffer)
{
  char rvalue = MMC_RESPONSE_ERROR;         // MMC_SUCCESS;
  //  char c = 0x00;

  // Set the block length to read
  if (mmcSetBlockLength (count) == MMC_SUCCESS)   // block length could be set
  {
    // CS = LOW (on)
    CS_LOW ();
    // send write command
    mmcSendCmd (MMC_WRITE_BLOCK,address, 0xFF);

    // check if the MMC acknowledged the write block command
    // it will do this by sending an affirmative response
    // in the R1 format (0x00 is no errors)
    if (mmcGetXXResponse(MMC_R1_RESPONSE) == MMC_R1_RESPONSE)
    {
      spiSendByte(DUMMY_CHAR);
      // send the data token to signify the start of the data
      spiSendByte(0xfe);
      // clock the actual data transfer and transmitt the bytes

      spiSendFrame(pBuffer, count);

      // put CRC bytes (not really needed by us, but required by MMC)
      spiSendByte(DUMMY_CHAR);
      spiSendByte(DUMMY_CHAR);
      // read the data response xxx0<status>1 : status 010: Data accected, status 101: Data
      //   rejected due to a crc error, status 110: Data rejected due to a Write error.
      mmcCheckBusy();
      rvalue = MMC_SUCCESS;
    }
    else
    {
      // the MMC never acknowledge the write command
      rvalue = MMC_RESPONSE_ERROR;   // 2
    }
  }
  else
  {
    rvalue = MMC_BLOCK_SET_ERROR;   // 1
  }
  // give the MMC the required clocks to finish up what ever it needs to do
  //  for (i = 0; i < 9; ++i)
  //    spiSendByte(0xff);

  CS_HIGH ();
  // Send 8 Clock pulses of delay.
  spiSendByte(DUMMY_CHAR);
  return rvalue;
} // mmc_write_block

