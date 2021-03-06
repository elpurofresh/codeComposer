/*
 *  ==== DO NOT MODIFY THIS FILE - CHANGES WILL BE OVERWRITTEN ====
 *
 *  Generated from
 *      C:/ti/grace_1_10_04_36/packages/ti/mcu/msp430/csl/communication/USCI_B0_init.xdt
 */

#include <msp430.h>

/*
 *  ======== USCI_B0_init ========
 *  Initialize Universal Serial Communication Interface B0 I2C 2xx
 */
void USCI_B0_init(void)
{
    /* Disable USCI */
    UCB0CTL1 |= UCSWRST;
    
    /* 
     * Control Register 0
     * 
     * ~UCA10 -- Own address is a 7-bit address
     * ~UCSLA10 -- Address slave with 7-bit address
     * ~UCMM -- Single master environment. There is no other master in the system. The address compare unit is disabled
     * ~UCMST -- Slave mode
     * UCMODE_3 -- I2C Mode
     * UCSYNC -- Synchronous Mode
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCB0CTL0 = UCMODE_3 + UCSYNC;
    
    /* 
     * I2C Own Address Register
     * 
     * ~UCGCEN -- Do not respond to a general call
     * 
     * Note: ~UCGCEN indicates that UCGCEN has value zero
     */
    UCB0I2COA = 72;
    
    /* Enable USCI */
    UCB0CTL1 &= ~UCSWRST;
}
