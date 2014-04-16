/*Main.C - LINO TECH
  Designed by Carter
  2008-03-19
*/

#include  <msp430x14x.h>
#include <in430.h>
#include  <string.h>
#include <stdio.h>

#include "xtype.h"
#include "SD.h"
#include "fat16.h"

#define SDCS (uint8)0x01   //P5_0

void delay(uint16 tick)
{
    while(tick-->0);
}

uint8 if_spiSend(hwInterface *iface, uint8 outgoing)
{
    uint8 incoming=0;	
  
    P5OUT &= ~SDCS;
    
    while (!(IFG2 & UTXIFG1));            // USART1 TX buffer ready?
    TXBUF1 = outgoing;
 
    while (!(IFG2 & URXIFG1));            // USART0 RX buffer ready?    
    incoming = RXBUF1; 
    
    P5OUT |= SDCS;
/*
    while(!(*pSPI_STAT & SPIF));
	*pSPI_TDBR = outgoing;

	while(*pSPI_STAT & RXS)
	incoming = *pSPI_RDBR;
*/
	return(incoming);	
}

void initMcu(void)
{
    WDTCTL = WDTPW + WDTHOLD;             // Stop WDT  

    BCSCTL1&=~XT2OFF;       //XT2 available         
    BCSCTL2|=SELM1;         //Mclk,SMCLK=8M
    BCSCTL2|=SELS; 

    do              
    { 
        int k = 0xff;
        IFG1 &= ~OFIFG;              
        for(k=0xff; k>0;k--);       
    }
    while((IFG1&OFIFG)!=0);
}

void main(void)
{ 
    hwInterface sdNow;  
	uint8 ok,i;
	uint8 Name[12]="SD_FAT16TXT";
	uint8 buffer[54]=
		"If you look this content,the file was created success!";    
    uint8 rBuf[128];
    
    initMcu(); 
    
    UCTL1 |= SWRST;                       // Initialize USART state machine
    UCTL1 |= CHAR + SYNC + MM;            // 8-bit SPI Master **SWRST**
    UTCTL1 = CKPH + SSEL1 + SSEL0 + STC;  // SMCLK, 3-pin mode
    UBR01 = 0x04;                         // UCLK/4
    UBR11 = 0x00;                         // 0
    UMCTL1 = 0x00;                        // no modulation
  
    ME2 |= USPIE1;                        // Enable USART1 SPI mode
    UCTL1 &= ~SWRST;                      // Initialize USART state machine   
  
    P5DIR |= SDCS;                        //SD cs pin as output 
    P5SEL |= 0x0E;                        // P5.1-3 SPI option select      
    
    sdNow.sectorCount = 1;
    sd_Init(&sdNow);
    
   	InitFat16();
    
//    memset(rBuf,0,128);
//    sd_readSector(&sdNow,0,rBuf,128);
//    rBuf[1] = 0;

	ok=CreateFile(Name,(uint32)54);				//建立长度为54的空文件
	if(ok!=SD_FAIL) 					//文件已存在,退出
    {
		WriteFile(Name,(uint32)0,(uint32)54,buffer);	//将buffer写入文件
    };
    
	/*
	for(i=0;i<54;i++)buffer[i]=0;
	ReadFile(Name,(uint32)0,(uint32)54,buffer);	//将文件读入buffer
	
	EreaseFile(Name);								//删除文件
	*/
    while(1);
}

