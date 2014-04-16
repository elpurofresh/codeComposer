#include "msp430fr5739.h"

void RTC_init(void);
void CS_init(void);

int botID = 1;
int sec = 0;
int min = 0;
int hrs = 0;
int timeToSend = 1;

void main(void){

	WDTCTL = WDTPW + WDTHOLD;

	PJDIR |= BIT0 + BIT2;
	PJOUT &= ~(BIT0 + BIT2);

	CS_init();
	RTC_init();

	_EINT();
	while(1){

		/*if(timeToSend){
			//_delay_cycles(64535632);
			//RTCSEC = RTCSEC + 0x02;
			//RTCMIN = RTCMIN + 0x03;
			//RTCHOUR = RTCHOUR + 0x01;
			PJOUT ^= BIT2;
			timeToSend = 0;
		}*/
	}

}

void RTC_init(void)
{
	RTCCTL01 |= RTCBCD + RTCHOLD + RTCRDYIE;	 // bcd mode; hold rtc for setting; enable rtc ready interrupt; 1sec
	RTCSEC = 0x00;
	RTCMIN = 0x00;
	RTCHOUR = 0x00;
	RTCCTL01 &= ~RTCHOLD;	 // release rtchold, begin count
}


// clock init
void CS_init(void)
{
	PJSEL0 |= BIT4 + BIT5;	 // XT1
	CSCTL0_H = 0xA5;                          // Unlock register
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
	CSCTL2 = SELA_0 + SELS_3 + SELM_3;        // set ACLK = XT1; MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
	CSCTL4 |= XT1DRIVE_0;
	CSCTL4 &= ~XT1OFF;

	do
	{
		CSCTL5 &= ~XT1OFFG;               // Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
	//CSCTL0_H = 0x01;                          // Lock Register
}


// RTC_b isr


#pragma vector=RTC_VECTOR
__interrupt void rtc_isr(void)
{

	switch(__even_in_range(RTCIV,0x12))
	{
	case RTCIV_NONE: break;               // No interrupt
	case RTCIV_RTCRDYIFG: // rtc ready
		while (!(RTCRDY));
		PJOUT ^= BIT0;
		sec++;

		//sec = RTCSEC;
		//min = RTCMIN;
		//hrs = RTCHOUR;

		/*if (sec >= 0x60) {
			sec = 0x00;
			min++;
		}

		if (min >= 0x60) {
			min = 0x00;
			min++;
		}

		if (hrs >= 24) {
			hrs = 0x00;
		}*/

		/*if (sec % 12 == botID) {
			timeToSend = 1;
		}*/

		break;

	case RTCIV_RTCTEVIFG: // rtc interval timer

		// do nothing here for now

		break;

	case RTCIV_RTCAIFG: break;         // rtc user alarm
	case RTCIV_RT0PSIFG: break;         // rtc prescaler 0
	case RTCIV_RT1PSIFG: break;         // rtc prescaler 1
	case RTCIV_RTCOFIFG: break;	 // rtc oscillator fault
	default: break;

	}

}
