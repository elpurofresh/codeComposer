#include <msp430.h>
#include "spi.h"

/* Initialize and enable the SPI module */
void spi_initialize( void )
{
  P2DIR |= 0x01;      // CS Line
  UCB0CTL1 |= UCSWRST;
  UCB0CTL0 |= UCMSB |  UCMST | UCSYNC;
  UCB0CTL1 = UCSSEL_2;
  P1SEL  |= 0xE0; // P1.5, P1.6, P1.7 option select
  P1SEL2 |= 0xE0;
  UCB0CTL1 &= ~UCSWRST;

  P2OUT |= (0x01);       // CS high 

}
  
  
char spi_send( char payload )
{
  IFG2 &= ~UCB0RXIFG;  
  //P2OUT &= ~(0x01); 
  UCB0TXBUF = payload;             
  while (!(IFG2 & UCB0RXIFG));  // USCI_B0 TX buffer ready?
  return UCB0RXBUF;
  //P2OUT |= (0x01);  
}

char spi_receive( void )
{
  IFG2 &= ~UCB0RXIFG;   
  //P2OUT &= ~(0x01);
  UCB0TXBUF = 0xFF; // Send dummy packet to get data back.
  while (!(IFG2 & UCB0RXIFG));      // USCI_B0 TX buffer ready?
  //P2OUT |= (0x01);  
  return UCB0RXBUF;
}