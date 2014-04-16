/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"
#include <stdio.h>
#include <stdlib.h>

void genPacket(void);
void txPacket(char *buffer);
void parsePacket(void);
void broadCast(void);
void savingToSdCard(void);
void timeKeeper(void);

void RTC_init(void);


void startTimer(void);
void stopTimer(void);
void to_lpm(void);

int sec = 0;
int min = 0;
int hrs = 0;
int numBotsData = 0;

int initCorrect = 0;

int cnterTest = 0;
char uart_Test[60] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
char buff[28] = "@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
int buffLen = 0;

char tsec1[2] = "$$", tmin1[2] = "$$", thrs1[2] = "$$", tPH1[2] = "$$";
int tsec2 = 0, tmin2 = 0, thrs2 = 0, tPH2 = 0;

int keepCounting = 1;

void main(void) {

	initClocks();
	initUARTSdCard();
	initUARTBot();

	initLEDs();
	//initLEDWait();	// Sleep for 5 sec: 5*8MHz.
	initUartBuffer();

	_EINT();

	/*initCorrect = checkSDCardStatus(); // Enables/Disables the main while loop

	if (initCorrect) {
		StartUpSequence();
	} else {
		blinkLED8(1);
	}*/
	initCorrect = 1;

	setupTimer();
	//RTC_init();

	OperationMode = Listening;

	P1OUT &= ~BIT4;                           //
	P1IES &= ~BIT4;                           // P1.4 Lo/Hi edge
	P1IE = BIT4;                              // P1.4 interrupt enabled
	P1IFG &= ~BIT4;                           // P1.4 IFG cleared


	while(initCorrect){

		/*if (P4IN & BIT0) {
			P3OUT ^= BIT4;
			sec++;
			P3OUT ^= BIT4;
		}*/

		__bis_SR_register(LPM3_bits + GIE);     // Enter LPM4 w/interrupt
		//__no_operation();			    // For debugger
		P1IE = BIT4;                            // P1.4 interrupt enabled

		switch (OperationMode) {

		case Broadcasting:
			broadCast();
			OperationMode = Listening;
			break;

		case Counting:
			startTimer();
			//to_lpm();
			break;

		case Listening:
			//stopTimer();
			//blinkLED3(1); //one second delay
			timeKeeper();
			//OperationMode = Counting;
			break;

		case Saving:
			savingToSdCard();
			break;

		default:
			OperationMode = Listening;
			break;
		}
	}
}

void genPacket(void)
{
	int tsec = 0, tmin = 0, thrs = 0, tPH = 0;
	numBotsData = 1;

	if ((sec - 9) <= 0) {
		tsec = sec + 60;
	} else {
		tsec = sec;
	}

	if ((min - 9) <= 0) {
		tmin = min + 60;
	} else {
		tmin = min;
	}

	thrs = hrs + 24;

	tPH = PH + 14;

	sprintf(buff, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", botID, is_parent, has_Children, is_leaf, tsec, tmin, thrs, sendFirstTime, numBotsData, botID, tsec, tmin, thrs, OX, tPH, TP, AB);
}

void txPacket(char *buffer)
{
	int i = 0, j = 0;

	for (i = 0; i < 10; ++i) {

		P3OUT ^= BIT7;
		UART_BotputChar(START_BYTE);

		for (j = 0; j < strlen(buffer); ++j) {  // Send 10 times the data packet with a 0.25sec pause in between
			UART_BotputChar(buffer[j]);
		}
		UART_BotputChar(END_BYTE);
		P3OUT ^= BIT7;

		sleepQuart();
	}
}

void parsePacket(void)
{
	sprintf(tsec1, "%c%c", uartBuffer[4], uartBuffer[5]);		// seconds of the network
	tsec2 = atoi(tsec1);
	if (tsec2 >= 60) {
		tsec2 = tsec2 - 60;
	}
	sec = tsec2;

	sprintf(tmin1, "%c%c", uartBuffer[6], uartBuffer[7]);		// minutes of the network
	tmin2 = atoi(tmin1);
	if (tmin2 >= 60) {
		tmin2 = tmin2 - 60;
	}
	min = tmin2;

	sprintf(thrs1, "%c%c", uartBuffer[8], uartBuffer[9]);		// hours of the network
	thrs2 = atoi(thrs1);
	hrs = thrs2 - 24;

	sprintf(tPH1, "%c%c", uartBuffer[20], uartBuffer[21]);  	// this will not work after more than one robot
	tPH2 = atoi(tPH1);
	tPH2 = tPH2 - 14;
}

void broadCast(void)
{
	genPacket();
	txPacket(buff);
	//OperationMode = Counting;
	OperationMode = Listening;
}

void savingToSdCard(void)			// Saves received data on to SD card and operates on the packet
{
	turnSdCard(ON);
	writeDataToSDFile("append myData.txt", uartBuffer, 1);
	turnSdCard(OFF);

	parsePacket();

	//startTimer();

	//OperationMode = Counting;
	OperationMode = Listening;
}

void timeKeeper(void)
{
	//blinkLED4(1);
	if (sec == 59) { sec = 0; min ++; }
	if ( (sec % 10) == 0 ) {
		OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }


	/*if (sec == 59) { sec = 0; min ++; }
	if ( sec == 10 || sec == 20 || sec == 30) {
		OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }*/
}

void RTC_init(void)
{
	RTCCTL01 |= RTCBCD + RTCHOLD + RTCRDYIE;	 // bcd mode; hold rtc for setting; enable rtc ready interrupt; 1sec
	RTCSEC = 0x00;
	RTCMIN = 0x00;
	RTCHOUR = 0x00;
	RTCCTL01 &= ~RTCHOLD;	 // release rtchold, begin count
}

void startTimer(void)
{
	TA0CTL |= MC_1; 		// Start Timer0 in continuous mode
	TA0CCTL0 |= CCIE;
}

void stopTimer(void)
{
	TA0CTL &= ~MC_1;		// Stop Timer0
	// Disable IE
	TA0CCTL0 &= ~CCIE;
	// Reset IRQ flag
	TA0CCTL0 &= ~CCIFG;
	TA0R = 0;				// Set Timer0 count register to 0x0000
}

void to_lpm(void)
{
	// Go to LPM3
	_BIS_SR(LPM3_bits + GIE);
	//__no_operation();
}

// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))	  // Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		P3OUT ^= BIT6;
		if (!pcktStartFlagBot && UCA0RXBUF == START_BYTE) {
			pcktStartFlagBot = 1;
			dataLength = 0;					  //Reset initial position in buffer (if no END_BYTE is seen at the end of package error will occur
		}
		else if (pcktStartFlagBot && UCA0RXBUF != END_BYTE) {
			uartBuffer[dataLength] = UCA0RXBUF;
			dataLength++;
		}
		else if (pcktStartFlagBot && UCA0RXBUF == END_BYTE) {
			pcktStartFlagBot = 0;			 //Reset framing
			OperationMode = Saving;			 //Set flag to operate on received data package
		}
		P3OUT ^= BIT6;
		break;

	case 4:break;                            // Vector 4 - TXIFG
	default: break;
	}
}

// Receives data from the SD card
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,0x08))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		while (!(UCTXIFG & UCA1IFG));           // USCI_A1 TX buffer ready?

		byteRX_SD = UCA1RXBUF;                     // Store RXed character

		uart_Test[cnterTest] = byteRX_SD;

		if (!pcktStartFlagSD && byteRX_SD == START_BYTE) {
			pcktStartFlagSD = 1;
			dataLength = 0;						// Reset initial position in buffer (if no END_BYTE is seen at the end of package error will occur
		}
		else if (pcktStartFlagSD && byteRX_SD != END_BYTE) {
			uartBuffer[dataLength] = byteRX_SD;
			dataLength++;
		}
		else if (pcktStartFlagSD && byteRX_SD == END_BYTE) {
			pcktStartFlagSD = 0;					// Reset framing
			lineCnter++;						// Increase number of lines
		}

		cnterTest++;
		if (cnterTest > 60) {
			cnterTest = 0;
		}

		break;
	case 4:break;                             // Vector 4 - TXIFG
	default: break;
	}
}

// After 37628 ticks, it increases the second counter
/*#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{
	sec++;  // Making the interrupt do something really small to eliminate the problem of the interbot UART not entering its interrupt
	PJOUT ^= BIT0;
	OperationMode = Listening;

	if (sec == 59) { sec = 0; min ++; }
	if ( sec == 10 || sec == 20 || sec == 30) {
		OperationMode = Broadcasting;			// Set flag to operate on received data package timeToSend = 1;
	}
	if (min == 59) { min = 0; hrs++;  }
	if (hrs == 23) { hrs = 0; 		  }

	PJOUT ^= BIT0;

	__bic_SR_register_on_exit(LPM3_bits+GIE);    // Restores active mode on return

}*/

/*// RTC_B Interrupt Service Routine
#pragma vector=RTC_VECTOR
__interrupt void rtc_isr(void)
{

	switch(__even_in_range(RTCIV,0x02)) //0x12
	{
	case RTCIV_NONE: 		break;      // No interrupt
	case RTCIV_RTCRDYIFG: 				// rtc ready

		while (!(RTCRDY));
		PJOUT ^= BIT3;
		sec++;
		break;

	case RTCIV_RTCTEVIFG: 	break;		// rtc interval timer
	case RTCIV_RTCAIFG: 	break;		// rtc user alarm
	case RTCIV_RT0PSIFG: 	break;		// rtc prescaler 0
	case RTCIV_RT1PSIFG: 	break;      // rtc prescaler 1
	case RTCIV_RTCOFIFG: 	break;		// rtc oscillator fault
	default: break;

	}
}*/

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	PJOUT ^= BIT3;                          // P1.0 = toggle
	sec++;
	P1IES ^= BIT4;
	P1IFG &= ~BIT4;                         // Clear P1.4 IFG
	P1IE = 0;
	__bic_SR_register_on_exit(LPM3_bits + GIE);     // Exit LPM4
}

