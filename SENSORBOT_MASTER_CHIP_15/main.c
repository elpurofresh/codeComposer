#include "msp430g2553.h"
#include "common_variables.h"
#include "common_functions.h"

int counterTemp = 0;
char uART_test[38] = "@@@@@@@@@@@@@@@@@@@@";
void runCommRoutine(void);
void transmitPacket(char *d, int s);
char endRxProcess = 0;

void main(void){

	configClock();
	setupUART();
	setupP2Pins();

	P2DIR |= CLK_Enable;
	P2OUT &= ~CLK_Enable;
	_EINT();

	myTimeSlot = 1;							//enabled once every time slot -- will be defined later
	storeDataFlag = 1;

	P2OUT |= TxRx_Enable;
	initUartBuffer();

	/*if (checkStatus(CLK_CHECK) && checkStatus(SD_CHECK)) {		//checks the status of all 3 modules
		myTimeSlot = 1;
	}

	else {
		blinkLEDRed(3);  //error
		myTimeSlot = 0;
	}*/

	while(myTimeSlot)
	{
		runCommRoutine();
	}
}

void runCommRoutine(void)
{
	P2OUT &= ~TxRx_Enable;
	OX = 65/*rand() % 100*/;
	amb = 43/*rand() % 100*/;
	temp = 21/*rand() % 100*/;
	PH = 01/*rand() % 15*/;
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
	blinkLEDRednGreen(1);*/
	for(i = 0; i < 30; i++){
		transmitPacket(dataIn, 19);
	}

	while(endRxProcess != 1){
		blinkLEDRed(4);
	}
	myTimeSlot = 0;
	blinkLEDRednGreen(2);
}

void transmitPacket(char *d, int s){
	volatile unsigned int j;
	P2OUT |= TxRx_Enable;
	UART_putChar(START_BYTE);
	UART_putChar('X');						//we don't know why we need this character but it is necessary to successfully send the correct packet
	for (j = 0; j < s; ++j) {
		UART_putChar(d[j]);
	}
	UART_putChar(END_BYTE);
	//P2OUT &= ~TxRx_Enable;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     	// Store RXed character

	uART_test[counterTemp] = byteRX;

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
			endRxProcess = 1;
		}

		else if (!pcktStartFlag && byteRX == '>') {
			storeDataInVar();
			storeDataFlag = 0;
		}

		counterTemp++;
		if (counterTemp > 38) {
			counterTemp = 0;
		}
	}
}
