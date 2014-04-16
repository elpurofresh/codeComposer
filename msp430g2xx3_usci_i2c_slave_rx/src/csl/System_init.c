/*
 *  ==== DO NOT MODIFY THIS FILE - CHANGES WILL BE OVERWRITTEN ====
 *
 *  Generated from
 *      C:/ti/grace_1_10_04_36/packages/ti/mcu/msp430/csl/system/System_init.xdt
 */

#include <msp430.h>

/*
 *  ======== System_init ========
 *  Initialize MSP430 Status Register
 */
void System_init(void)
{
    /* 
     * IFG2, Interrupt Flag Register 2
     * 
     * ~UCB0TXIFG -- No interrupt pending
     * UCB0RXIFG -- Interrupt pending
     * ~UCA0TXIFG -- No interrupt pending
     * ~UCA0RXIFG -- No interrupt pending
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    IFG2 &= ~(UCB0RXIFG);

    /* 
     * IE2, Interrupt Enable Register 2
     * 
     * ~UCB0TXIE -- Interrupt disabled
     * UCB0RXIE -- Interrupt enabled
     * ~UCA0TXIE -- Interrupt disabled
     * ~UCA0RXIE -- Interrupt disabled
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    IE2 |= UCB0RXIE;

}
