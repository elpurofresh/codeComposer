#include "CTS_Layer.h"

unsigned int proximityCnt;
unsigned int buttonCnt;
unsigned int wheelCnt[4];

void main(void)
{   
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  BCSCTL1 = CALBC1_1MHZ;                // Set DCO to 1, 8, 12 or 16MHz
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 |= DIVS_2;                    // divide SMCLK by 4 for 2Mhz    
  BCSCTL3 |= LFXT1S_2;                  // LFXT1 = VLO
  
  P1DIR = 0xFF;
  P1OUT = 0;
  P2DIR = 0xFF;
  P2OUT = 0;
  P2SEL = 0;
  
  while (1)
  {
      TI_CAPT_Raw(&proximity_sensor,&proximityCnt);
      TI_CAPT_Raw(&middle_button,&buttonCnt);
      TI_CAPT_Raw(&wheel,wheelCnt);
      __no_operation();
  }
}
