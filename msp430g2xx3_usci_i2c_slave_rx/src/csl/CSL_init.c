/*
 *  ======== CSL_init.c ========
 *  DO NOT MODIFY THIS FILE - CHANGES WILL BE OVERWRITTEN
 */
 
/* external peripheral initialization functions */
extern void GPIO_init(void);
extern void BCSplus_init(void);
extern void USCI_B0_init(void);
extern void System_init(void);
extern void WDTplus_init(void);

#include <msp430.h>

/* verify that the MSP430 headers included support the code that's generated */
#if defined(__TI_COMPILER_VERSION__)
  /* pragma required to suppress TI warning that #warning is unrecognized */
  #pragma diag_suppress 11
#endif
#if __MSP430_HEADER_VERSION__ < 1062
  #if defined(__TI_COMPILER_VERSION__)
    #warn The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1062, please update your version of the msp430 headers.
  #elif defined(__GNUC__) || defined(__IAR_SYSTEMS_ICC__)
    #warning The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1062, please update your version of the msp430 headers.
  #else
    /* if we can't just warn, resort to ANSI C's #error */
    #error The MSP430 headers included may be incompatible with the generated source files.  If the value of __MSP430_HEADER_VERSION__, declared by msp430.h, is less than 1062, please update your version of the msp430 headers.
  #endif
#endif
#if defined(__TI_COMPILER_VERSION__)
  /* pragma required to restore TI warnings about unrecognized directives */
  #pragma diag_default 11
#endif

/*
 *  ======== CSL_init =========
 *  Initialize all configured CSL peripherals
 */
void CSL_init(void)
{
    /* Stop watchdog timer from timing out during initial start-up. */
    WDTCTL = WDTPW + WDTHOLD;

    /* initialize Config for the MSP430 GPIO */
    GPIO_init();

    /* initialize Config for the MSP430 2xx family clock systems (BCS) */
    BCSplus_init();

    /* initialize Config for the MSP430 USCI_B0 */
    USCI_B0_init();

    /* initialize Config for the MSP430 System Registers */
    System_init();

    /* initialize Config for the MSP430 WDT+ */
    WDTplus_init();

}

/*
 *  ======== Interrupt Function Definitions ========
 */

/* Interrupt Function Prototypes */
extern void USCIAB0RX_ISR(void);






/*
 *  ======== USCI A0/B0 TX Interrupt Handler Generation ========
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR_HOOK(void)
{

	/* USCI_B0 Receive Interrupt Handler */
	USCIAB0RX_ISR();

	/* Enter active mode on exit */
	__bic_SR_register_on_exit(LPM4_bits);
}
