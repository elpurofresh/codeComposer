
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
#define KEYCODESIZE 7
#define MYDATASIZE KEYCODESIZE+2

void configCLK(void);
void configUART(void);
void configTIMER(void);


void onSequence(void);
void startCLK(void);
void stopCLK(void);
void startRx(void);
void stopRx(void);
void resetInputBuffer(void);
void txPacket(char *buffer);
void uart_PutCharOut(char charToSend);
void checkMsg(char* msg);
int compChars(char* c1, char* c2, int len);

void configADC(void);
void startADC(void);
void stopADC(void);
void itoa(int value, char s[]);
void readTemp(void);

void sleepQuart(void);						//delays the processor for 1/4 sec

volatile int dataLengthOut = 0;
char myKeyCode[KEYCODESIZE] = "SENDC1\0";
char keyCode_X[KEYCODESIZE] = "SENDC2\0";
char KeyCode_Y[KEYCODESIZE] = "SENDC3\0";
char myData[MYDATASIZE]		= "SENDC1##\0";
char reqChild[KEYCODESIZE]	= "REQTCX\0";
char childCode[KEYCODESIZE] = "DTC424\0";

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

//volatile unsigned int ADC_Result;
volatile unsigned int tempVal;


void main(void)
{
	configCLK();
	configUART();
	configTIMER();
	configADC();


	onSequence();

	startRx();
	_EINT();

	while(1){


		if (rxd) {
			checkMsg(uartRxBuff);

			if (masterRequest /*&& clkTimer == 2*/) {//timeFlag //after one sec
				masterRequest = FALSE;
				stopRx();
				startADC();
				readTemp();
				stopADC();
				//txPacket(myKeyCode);
				txPacket(myData);
				_delay_cycles(2000000);
				txPacket(reqChild);
				_delay_cycles(2000000);
				startRx();
				resetInputBuffer();
				sendDataToChild = TRUE;
				rxd = FALSE;
			}

			else if (sendDataToMaster /*&& clkTimer == 4*/) { //timeFlag //within 2 secs
				sendDataToMaster = FALSE;
				stopCLK();
				stopRx();
				uartRxBuff[0] = 'C';
				uartRxBuff[1] = '1';
				uartRxBuff[dataLenToSend] = '\0';
				txPacket(uartRxBuff);
				_delay_cycles(2000000);//8500000
				startRx();
				resetInputBuffer();
				firstTime = TRUE;
				sendDataToChild = FALSE;
				rxd = FALSE;
			}
		}


		if (sendDataToChild /*!sendDataToMaster*/ && clkTimer > 4){ //(!sendDataToMaster && sentACKToMaster && clkTimer == 6) {
			sentACKToMaster = FALSE;
			stopCLK();
			stopRx();
			txPacket(myData);
			_delay_cycles(2000000);
			startRx();
			resetInputBuffer();
			firstTime = TRUE;
			sendDataToChild = FALSE;
			rxd = FALSE;
		}

		if (otherRequest /*&& clkTimer == 1*/) {
			otherRequest = FALSE;
			stopCLK();
			stopRx();
			_delay_cycles(24000000); //wait one interval
			startRx();
			resetInputBuffer();
			firstTime = TRUE;
			sendDataToChild = FALSE;
			rxd = FALSE;
		}

		_low_power_mode_3();
	}
}

void configCLK(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // stop watchdog

	CSCTL0_H = 0xA5;
	CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting: 8MHz, DCOSEL = 0 if 1, then DCO = 24MHz
	CSCTL2 = SELS_3 + SELM_3;        // set ACLK = SMCLK; MCLK = DCO
	CSCTL3 = DIVS_0 + DIVM_0;        // set all dividers
}

void configUART(void)
{
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
}

void configTIMER(void)
{
	//TA0CCTL0 |= CCIE;
	TA0CCR0 = 65536-1;					// CCR0 counter
	TA0CTL = TASSEL_2 + ID_3 + MC_1;	// SMCLK, f(SMCLK)/8, UP mode
	TA0EX0 = TAIDEX_7;					// Input divider expansion (f(SMCLK)/x)/8
}

void configADC(void)
{
	// Configure ADC
	P1SEL1 |= BIT3;
	P1SEL0 |= BIT3;

	ADC10CTL0 |= ADC10SHT_2 + ADC10ON;        // ADC10ON, S&H=16 ADC clks
	ADC10CTL1 |= ADC10SHP;                    // ADCCLK = MODOSC; sampling timer
	ADC10CTL2 |= ADC10RES;                    // 10-bit conversion results
	ADC10MCTL0 |= ADC10INCH_3;                // A1 ADC input select; Vref=AVCC
	ADC10IE |= ADC10IE0;                      // Enable ADC conv complete interrupt
}

void startADC(void)
{
	ADC10CTL0 |= ADC10ENC + ADC10SC;        // Sampling and conversion start
}

void stopADC(void)
{
	ADC10CTL0 &= ~(ADC10ENC + ADC10SC);        // Sampling and conversion stop
}

void itoa(int value, char s[])
{
	int i = strlen(s); // MAX_NUM_DIGITS = 2

	do {
		s[--i] = value % 10 + '0';

	} while ( (value /= 10) > 0);
}

void readTemp(void)
{
	char data[3] = "@@\0";
	itoa(tempVal, data); //SENDC1##\0
	myData[MYDATASIZE-3] = data[0];
	myData[MYDATASIZE-2] = data[1];
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

	uart_PutCharOut('<');

	for (i = 0; i < strlen(buffer); ++i) {
		uart_PutCharOut(buffer[i]);
	}

	uart_PutCharOut('>');
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

void checkMsg(char* msg)
{

	/*if (!strcmp(msg, myKeyCode)) {
		masterRequest = TRUE;
	} else if (!strncmp(msg,childCode,2)) {
		sendDataToMaster = TRUE;
	} else {
		otherRequest = TRUE;
	}*/

	if (compChars(msg, myKeyCode, 0)) {
		masterRequest = TRUE;
	} else if (compChars(msg, childCode, 4)) {
		sendDataToMaster = TRUE;
	} else {
		otherRequest = TRUE;
	}

}

int compChars(char* c1, char* c2, int len)
{
	int i = 0;

	if (len == 0) {
		for (i = 0; i < strlen(c2); ++i) {
			if (c1[i] != c2[i]) {
				return FALSE;
			}
		}
		return TRUE;
	} else {
		for (i = 0; i < len; ++i) {
			if (c1[i] != c2[i]) {
				return FALSE;
			}
		}
		return TRUE;
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
		testData[testCounter++]=UCA0RXBUF;
		//uartRxBuff[dataLengthIn++] = UCA0RXBUF;

		if (!startFlag && UCA0RXBUF == '<') {
			startFlag = TRUE;
			dataLengthIn = 0;
		} else if (startFlag && UCA0RXBUF != '>') {
			uartRxBuff[dataLengthIn++] = UCA0RXBUF;
		} else if (startFlag && UCA0RXBUF == '>') {
			dataLenToSend = dataLengthIn;
			startFlag = FALSE;
			rxd = TRUE;
		}


		if (firstTime) {
			startCLK();
			firstTime = FALSE;
		}

		if (testCounter > 20) {
			testCounter = 0;
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
	//testCounter++;
	/*if (clkTimer == 2) {
		timeFlag = TRUE;
		clkTimer = 0;
	}*/
	__low_power_mode_off_on_exit();
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	switch(__even_in_range(ADC10IV,12))
	{
	case  0: break;                          // No interrupt
	case  2: break;                          // conversion result overflow
	case  4: break;                          // conversion time overflow
	case  6: break;                          // ADC10HI
	case  8: break;                          // ADC10LO
	case 10: break;                          // ADC10IN
	case 12: //ADC_Result = ADC10MEM0;
		tempVal = -0.2555*ADC10MEM0 + 111;
		//__low_power_mode_off_on_exit();	//__bic_SR_register_on_exit(CPUOFF);
		break;                          // Clear CPUOFF bit from 0(SR)
	default: break;
	}
}
