/*
 * hardware_board.h
 *
 *  Created on: Mar 26, 2012
 *      Author: andres
 */

//            MSP430G2553                MMC Card
//         -----------------          -----------------
//     /|\|              XIN|-   /|\ |                 |
//      | |                 |     |  |                 |
//      --|RST          XOUT|-    |--|Pin5/Vcc         |
//        |                 |        |                 |
//        |                 |        |                 |
//        |            P1.0 |<-------|Pin1/CD          |
//        |            P2.0 |------->|Pin7/CS          |
//        |                 |        |                 |
//        |      P1.1/SOMI1 |------->|Pin2/DO          |
//        |      P1.2/SIMO1 |<-------|Pin6/DI          |
//        |      P1.4/UCLK1 |------->|Pin4/CLK         |
//        |                 |        |                 |
//        |                 |     |--|Pin3/GND         |
//                                |
//                                =

//  Pin configuration at MSP430G2553:
//  --------------------------------
//  MSP430F169      MSP Pin        MMC             MMC Pin
//  -------------------------------------------------------------
//  P1.0             2			CardDetect       1
//  P1.1 /SOMI       3        	DataOut          2
//                   20		   	GND              3 (0 V)
//  P1.4 UCLK        6        	Signal CLK       4 (3.3 V)
//  			     1        	Vcc	             5
//  P1.2 /SIMO       4        	DataIn		     6
//  P2.0 			 8         	ChipSelect       7
//  -------------------------------------------------------------

#include <msp430g2553.h>              // Adjust this according to the
                                     // MSP430 device being used.
// SPI port definitions              // Adjust the values for the chosen
#define SPI_PxSEL         P1SEL      // interfaces, according to the pin
#define SPI_PxDIR         P1DIR      // assignments indicated in the
#define SPI_PxIN          P1IN       // chosen MSP430 device datasheet.
#define SPI_PxOUT         P1OUT
#define SPI_SIMO          0x02
#define SPI_SOMI          0x04
#define SPI_UCLK          0x08

// Chip Select
#define MMC_CS_PxOUT      P2OUT
#define MMC_CS_PxDIR      P2DIR
#define MMC_CS            0x01


// Card Detect
#define MMC_CD_PxIN       P1IN
#define MMC_CD_PxDIR      P1DIR
#define MMC_CD            0x40

#define CS_LOW()    MMC_CS_PxOUT &= ~MMC_CS               // Card Select
#define CS_HIGH()   while(!halSPITXDONE); MMC_CS_PxOUT |= MMC_CS  // Card Deselect

#define DUMMY_CHAR 0xFF

 #define halSPIRXBUF  UCA0RXBUF
 #define halSPI_SEND(x) UCA0TXBUF=x
 #define halSPITXREADY  (IFG1&UTXIFG0)         /* Wait for TX to be ready */
 #define halSPITXDONE  (UCA0STAT&UCBUSY)       /* Wait for TX to finish */
 #define halSPIRXREADY (IFG1&URXIFG0)          /* Wait for TX to be ready */
 #define halSPIRXFG_CLR IFG1 &= ~URXIFG0
 #define halSPI_PxIN  SPI_USART0_PxIN
 #define halSPI_SOMI  SPI_USART0_SOMI

// *************************************************************************************
//	For SD Card (MMC.h)
// *************************************************************************************



// error/success codes
#define MMC_SUCCESS           0x00
#define MMC_INIT_ERROR        0x04


// commands: first bit 0 (start bit), second 1 (transmission bit); CMD-number + 0ffsett 0x40
#define MMC_GO_IDLE_STATE          0x40     //CMD0
#define MMC_SEND_OP_COND           0x41     //CMD1


// read a size Byte big block beginning at the address.
char mmcReadBlock(const unsigned long address, const unsigned long count, unsigned char *pBuffer);
#define mmcReadSector(sector, pBuffer) mmcReadBlock(sector*512ul, 512, pBuffer)

// write a 512 Byte big block beginning at the (aligned) address
char mmcWriteBlock (const unsigned long address, const unsigned long count, unsigned char *pBuffer);
#define mmcWriteSector(sector, pBuffer) mmcWriteBlock(sector*512ul, 512, pBuffer)

unsigned char spiSendFrame(unsigned char* pBuffer, unsigned int size);
