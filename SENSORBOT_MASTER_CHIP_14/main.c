#include "msp430g2553.h"
#include "common_variables.h"
#include "common_functions.h"

#define ts			0
#define sec			1
#define ox			3
#define ph			4
#define te			5
#define am	 		6

int botID = 1;
int is_parent = 0;
int has_Children = 0;
int is_leaf = 1;
char sendFirsttime = '0';

int PH = 0;
int OX = 0;
int temp = 0;
int amb = 0;

int sec_0 = 0;
int sec_1 = 0;
int sec_2 = 0;
char header[] = "ID P HC IL TS-S SFT OX PH TE AM";

int timeSlot = 0;
long int secStamp = 0;

volatile unsigned int i;


char data[3] = "$$$";
int time;
char packet[];

int getTime(void);
void runCommRoutine(void);
int SDPacket(int I, int S, int E, int O, int P, int T, int A);
void TXPacket(int data, int address);
void transmitPacket(char *P, int s);
void itoa(int n, char s[]);
void reverse(char *string);
int string_length(char *pointer);
char processData(char *d, int s);
void loadData(char *d, int address);

void main(void){

	P2DIR |= BIT1;
	P2OUT &= ~BIT1;

	configClock();
	setupUART();

	P2DIR |= CLK_Enable;
	P2OUT &= ~CLK_Enable;
	_EINT();

	myTimeSlot = 1;							//enabled once every time slot -- will be defined later

	while(myTimeSlot)
	{

		P2OUT &= ~BIT1;
		initUartBuffer();
		if (checkStatus(CLK_CHECK) && checkStatus(SD_CHECK)) {		//checks the status of all 3 modules
			runCommRoutine();
		} else {
			blinkLEDRed(3);  //error
			myTimeSlot = 0;
		}
	}
}

void runCommRoutine(void)
{
	OX = rand() % 100;
	amb = rand() % 100;
	temp = rand() % 100;
	PH = rand() % 15;
	getTime();
	sleep();


	itoa(botID, dataIn);
	itoa(is_parent, dataIn + 1);
	itoa(has_Children, dataIn + 2);
	itoa(is_leaf, dataIn + 3);

	itoa(timeSlot, data);
	processData(data, 3);
	loadData(data, ts);

	itoa(secStamp, data);
	processData(data, 3);
	loadData(data, sec);

	dataIn[10] = sendFirsttime;

	itoa(OX, data);
	processData(data, 2);
	loadData(data, ox);

	itoa(PH, data);
	processData(data, 2);
	loadData(data, ph);

	itoa(temp, data);
	processData(data, 2);
	loadData(data, te);

	itoa(amb, data);
	processData(data,2);
	loadData(data, am);

	/*SDPacket(botID, timeSlot, secStamp, OX, PH, temp, amb);
	turnONSdCard();
	sleep();
	sendEscapeCmd();
	createNewFile("new myData.txt");
	sleep();
	writeDataToFile("append myData.txt", header, 0);
	sleep();
	writeDataToFile("append myData.txt", packet, 2);
	sleep();
	turnOFFSdCard();
	transmitPacket(packet, 15);
	blinkLEDRednGreen(1);*/
	myTimeSlot = 0;

}

int getTime(void)
{
	int i = 0, timeout = 10;
	byteRX = 0;

	P2OUT |= CLK_Enable;

	while(i < timeout){
		UART_putChar('Q');
		UART_putChar(CR);

		if (byteRX == 'J') {
			sleepHalf();
			timeSlot = byteRX;
			sleepHalf();
			sec_0 = byteRX;
			sleepHalf();
			sec_1 = byteRX;
			sleepHalf();
			sec_2 = byteRX;
			blinkLEDRednGreen(1);
			secStamp = sec_0 + sec_1 + sec_2;
			i = timeout;
		}

		blinkLEDRed(1);
		i++;
	}

	P2OUT &= ~CLK_Enable;

	return secStamp;
}


/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
	int i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do {       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void reverse(char *string)
{
	int length, c;
	char *begin, *end, temp;

	length = string_length(string);

	begin = string;
	end = string;

	for ( c = 0 ; c < ( length - 1 ) ; c++ )
		end++;

	for ( c = 0 ; c < length/2 ; c++ )
	{
		temp = *end;
		*end = *begin;
		*begin = temp;

		begin++;
		end--;
	}
}

int string_length(char *pointer)
{
	int c = 0;

	while( *(pointer+c) != '\0' )
		c++;

	return c;
}

char processData(char *d, int s){

	volatile unsigned int n = 0;
	volatile unsigned int check = 0;

	if (s == 3) {

		if(d[2] < 0x30 || d[2] >0x39){
			n = n + 1;
		}

		if (d[1] < 0x30 || d[1] >0x39) {
			n = n + 1;
		}

		if(n == 2){
			d[2] = d[2-n];
			d[0] = '0';
			d[1] = '0';
		}

		else if (n == 1) {
			d[2] = d[2-n];
			d[1] = d[1-n];
			d[1-n] = '0';
		}

	}

	if (s == 2) {
		if (d[1] < 0x30 || d[1] >0x39) {
			d[1] = d[0];
			d[0] = '0';
		}
	}

	return *d;
}

void loadData(char *d, int address){

	switch(address){

	case ts:
		for (i = 4; i < 7; ++i) {
			dataIn[i] = d[i-4];
		}
		break;

	case sec:
		for (i = 7; i < 10; ++i) {
			dataIn[i] = d[i-7];
		}
		break;

	case ox:
		for (i = 11; i < 13; ++i) {
			dataIn[i] = d[i-11];
		}
		break;

	case ph:
		for (i = 13; i < 15; ++i) {
			dataIn[i] = d[i-13];
		}
		break;

	case te:
		for (i = 15; i < 17; ++i) {
			dataIn[i] = d[i-15];
		}
		break;

	case am:
		for (i = 17; i < 19; ++i) {
			dataIn[i] = d[i-17];
		}
		break;
	}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     	// Store RXed character

	if (storeDataFlag == 1) {

		if (dataLength == MAX_Temp_Buffer-1) {
			dataLength = 0;
		}

		if (!pcktStartFlag && byteRX == START_BYTE) {
			pcktStartFlag = 1;
			dataLength = 0;
		}
		else if (pcktStartFlag && byteRX != END_BYTE) {
			uartBuffer[dataLength] = byteRX;
			dataLength++;
		}
		else if (pcktStartFlag && byteRX == END_BYTE) {
			pcktStartFlag = 0;
			lineCnter++;
		}

		else if (!pcktStartFlag && byteRX == '>') {
			storeDataInVar();
			storeDataFlag = 0;
		}
	}
}
