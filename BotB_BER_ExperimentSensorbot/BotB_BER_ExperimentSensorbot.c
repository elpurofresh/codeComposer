
#include "msp430fr5739.h"
#include <string.h>

#define UART_RXBot	BIT1		// RXD on P2.1
#define UART_TXBot	BIT0        // TXD on P2.0
#define TRUE		1
#define FALSE		0
#define START_BYTE  0x5B		// Refer to '[' in ASCII Matrix
#define END_BYTE	0x5D		// Refer to ']' in ASCII Matrix
#define END_TX	0xD 			// '\n'
#define BUFF_SIZE	37			// Max buffers size= data + internal delimiter '\0'
#define KEYCODESIZE 9

void onSequence(void);
void startCLK(void);
void stopCLK(void);
void startRx(void);
void stopRx(void);
void resetInputBuffer(void);
void txPacket(char *buffer);
void uart_PutCharOut(char charToSend);

void sleepQuart(void);						//delays the processor for 1/4 sec

volatile int dataLengthOut = 0;
char myKeyCode[KEYCODESIZE] = "<SENDC1>\0";
char keyCode_X[KEYCODESIZE] = "<SENDC2>\0";
char KeyCode_Y[KEYCODESIZE] = "<SENDC3>\0";
char myData[KEYCODESIZE+2]	= "<SENDC111>";
char reqChild[KEYCODESIZE]	= "<REQTCX>\0";
char childCode[KEYCODESIZE-2] = "GC424!";

char testData[BUFF_SIZE]	= "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0";//"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
char uartRxBuff[BUFF_SIZE] 	= "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0";
char uart_Out[BUFF_SIZE+1] 	= "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\0"; //includes termination char '!'
volatile int rxd = FALSE, timeFlag = FALSE;
volatile int startFlag = FALSE;
volatile int dataLengthIn = 0, clkTimer = 0, testCounter = 0;
volatile int flagACKTx = FALSE, flagRxMode = FALSE, firstTime = TRUE;

volatile int myKeyCounter = 0, keyCounter_X = 0, keyCounter_Y = 0;
volatile int masterRequest = FALSE, sendDataToMaster = FALSE, toMaster = FALSE;
volatile int sendDataToChild = FALSE;
volatile int sentACKToMaster = FALSE;
volatile int otherRequest = FALSE;
volatile int dataLenToSend = 0;
volatile int keyCounter_Req = 0;


void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELS_3 + SELM_3;        // set ACLK = SMCLK; MCLK = DCO
	CSCTL3 = DIVS_0 + DIVM_0;        // set all dividers

	// Configure UART 0
	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK = 8MHz as UCBRCLK
	UCA0BR0 = 52;                           	// (8000000Hz / 9600bps) / 16  = 52.083
	UCA0BR1 = 0;
	// OS16 enabled because we are using SMCLK @ 8MHz High Freq.
	// 52.083-52 = 0.083 *16 = 1.33
	// UCBRSx value 0.3335= 0x42 (See UG)
	UCA0MCTLW = 0x4911 ;						// UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide)
	UCA0CTL1 &= ~UCSWRST;                     	// Release from reset

	// Configure UART pins
	P2SEL1 |= UART_RXBot + UART_TXBot;
	P2SEL0 &= ~(UART_RXBot + UART_TXBot);

	P1DIR |= 0xFF; //(BIT0 + BIT1 + BIT2 + BIT5);
	P1OUT &= 0; //~(BIT0 + BIT1 + BIT2 + BIT5);

	//TA0CCTL0 |= CCIE;
	TA0CCR0 = 65536-1;					// CCR0 counter
	TA0CTL = TASSEL_2 + ID_3 + MC_1;	// SMCLK, f(SMCLK)/8, UP mode
	TA0EX0 = TAIDEX_7;					// Input divider expansion (f(SMCLK)/x)/8

	onSequence();

	startRx();
	_EINT();

	while(1){

		if (masterRequest /*&& clkTimer == 2*/) {//timeFlag //after one sec
			masterRequest = FALSE;
			//clkTimer = 0; //timeFlag = FALSE;
			//stopCLK();
			stopRx();
			//toMaster = TRUE; //allows reply to master
			txPacket(myKeyCode);
			_delay_cycles(2000000);
			//toMaster = FALSE; //avoids reply to master
			txPacket(reqChild);
			_delay_cycles(2000000);
			startRx();
			resetInputBuffer();
			//firstTime = TRUE;
			sendDataToChild = TRUE;
			//sentACKToMaster = TRUE;
		}

		if (sendDataToMaster /*&& clkTimer == 4*/) { //timeFlag //within 2 secs
			sendDataToMaster = FALSE;
			//clkTimer = 0; //timeFlag = FALSE;
			stopCLK();
			stopRx();
			//toMaster = TRUE;
			uartRxBuff[0] = '<';
			uartRxBuff[dataLenToSend] = '>';//uartRxBuff[BUFF_SIZE-2] = '>';
			uartRxBuff[dataLenToSend+1] = '\0';//uartRxBuff[BUFF_SIZE-1] = '\0';
			txPacket(uartRxBuff);
			_delay_cycles(2000000);//8500000
			startRx();
			resetInputBuffer();
			firstTime = TRUE;
			sendDataToChild = FALSE;
		}

		if (!sendDataToMaster && clkTimer > 4){ //(!sendDataToMaster && sentACKToMaster && clkTimer == 6) {
			sentACKToMaster = FALSE;
			//clkTimer = 0; //timeFlag = FALSE;
			stopCLK();
			stopRx();
			txPacket(myData);
			_delay_cycles(2000000);
			startRx();
			resetInputBuffer();
			firstTime = TRUE;
			sendDataToChild = FALSE;
		}

		if (otherRequest && clkTimer == 1) {
			otherRequest = FALSE;
			stopCLK();
			stopRx();
			_delay_cycles(4000000); //wait one interval
			startRx();
			resetInputBuffer();
			firstTime = TRUE;
			sendDataToChild = FALSE;
		}

		_low_power_mode_3();
	}
}

void onSequence(void)
{
	P1OUT |= BIT0;				//Red LED
	_delay_cycles(3500000);
	P1OUT &= ~BIT0;
	P1OUT |= BIT1;				//Yellow LED
	_delay_cycles(3500000);
	P1OUT &= ~BIT1;
	P1OUT |= BIT2;
	//_delay_cycles(3500000);		//Green LED
	//P1OUT &= ~BIT2;
}

void sleepQuart(void)
{
	_delay_cycles(2000000);
}

void startRx(void)
{
	UCA0IE |= UCRXIE;                         	// Enable RX interrupt
}

void stopRx(void)
{
	UCA0IE &= ~UCRXIE;                         	// Enable RX interrupt
}

void startCLK(void)
{
	TA0CCTL0 |= CCIE;
	TA0CCR0 = 65536-1;
}

void stopCLK(void)
{
	TA0CCTL0 &= ~CCIE;
	TA0CCR0 = 0;
	clkTimer = 0;
}

void resetInputBuffer(void)
{
	int i = BUFF_SIZE;
	dataLengthIn = 0;

	for (i = 0; i < BUFF_SIZE; ++i) {
		uartRxBuff[i] = '@';
	}
	uartRxBuff[BUFF_SIZE-1] = '\0';
}

void txPacket(char *buffer)
{
	static int i = 0;

	/*if (toMaster == TRUE) {
		uart_PutCharOut('(');
	}*/

	for (i = 0; i < strlen(buffer); ++i) {
		uart_PutCharOut(buffer[i]);
	}

	/*if (toMaster == TRUE) {
		uart_PutCharOut(')');
	}*/
}

void uart_PutCharOut(char charToSend)  	// UART0 Transmit Subroutine
{
	while (!(UCTXIFG & UCA0IFG));  // USCI_A0 TX buffer ready?
	UCA0TXBUF = charToSend;       // send data

	uart_Out[dataLengthOut++] = UCA0TXBUF;

	if (dataLengthOut > BUFF_SIZE-1) {
		dataLengthOut = 0;
	}
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG
		testData[dataLengthIn]=UCA0RXBUF;
		uartRxBuff[dataLengthIn++] = UCA0RXBUF;

		if (UCA0RXBUF == myKeyCode[myKeyCounter]) {
			myKeyCounter++;
			if (myKeyCounter == (KEYCODESIZE-1)) {
				myKeyCounter = 0;
				masterRequest = TRUE;
			}

		} /*else if (UCA0RXBUF == childCode[keyCounter_X]) {
			keyCounter_X++;
			if (keyCounter_X == (6)) {
				keyCounter_X = 0;
				sendDataToMaster = TRUE;
				dataLenToSend = dataLengthIn;
			}
		} else {
			otherRequest = TRUE;
		}*/

		if (UCA0RXBUF == keyCode_X[keyCounter_X]) {
			keyCounter_X++;
			if (keyCounter_X == (KEYCODESIZE-1)) {
				keyCounter_X = 0;
				otherRequest = TRUE;
			}
		}

		if (UCA0RXBUF == KeyCode_Y[keyCounter_Y]) {
			keyCounter_Y++;
			if (keyCounter_Y == (KEYCODESIZE-1)) {
				keyCounter_Y = 0;
				otherRequest = TRUE;
			}
		}

		if (UCA0RXBUF == reqChild[keyCounter_Req]) {
			keyCounter_Req++;
			if (keyCounter_Req == (KEYCODESIZE-1)) {
				keyCounter_Req = 0;
				otherRequest = TRUE;
			}
		}

		if (UCA0RXBUF == '!' && sendDataToChild == TRUE) {
			sendDataToMaster = TRUE;
			dataLenToSend = dataLengthIn;
		}

		if (firstTime) {
			startCLK();
			firstTime = FALSE;
		}

		if (dataLengthIn > BUFF_SIZE-1) {
			dataLengthIn = 0;
		}


		break;
	default: break;
	}
	__low_power_mode_off_on_exit();
}

// Timer A0 interrupt service routine
// Interrupt is fired every 0.5 secs
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	clkTimer++;
	testCounter++;
	/*if (clkTimer == 2) {
		timeFlag = TRUE;
		clkTimer = 0;
	}*/
	__low_power_mode_off_on_exit();
}
