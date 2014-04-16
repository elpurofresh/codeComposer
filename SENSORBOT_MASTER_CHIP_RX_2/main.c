#include "msp430g2553.h"
#include "common_variables.h"
#include "common_functions.h"

int  counterTemp  = 0;
char uART_test[38] = "@@@@@@@@@@@@@@@@@@@@";
char dataCheck[20] = "X1234567898765012143";
void runCommRoutine(void);
void transmitPacket(char *d, int s);
char checkPacket(char *d);


void main(void){

	configClock();
	setupUART();
	setupP2Pins();

	P2DIR |= CLK_Enable;
	P2OUT &= ~CLK_Enable;
	_EINT();

	myTimeSlot = 1;							//enabled once every time slot -- will be defined later
	storeDataFlag = 1;


	while(myTimeSlot)
	{
		P2OUT |= Rx_Enable;
		initUartBuffer();
		//runCommRoutine();
		blinkLEDGreen(1);

	}
}

void runCommRoutine(void)
{
	P2OUT &= ~Rx_Enable;
	P2OUT &= ~Tx_Enable;
	OX = 23/*rand() % 100*/;
	PH = 45/* rand() % 15*/;
	temp = 67/*rand() % 100*/;
	amb = 89/*rand() % 100*/;



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
	for (i = 0; i < 40; ++i) {
		transmitPacket(dataIn, 19);
	}
	myTimeSlot = 0;


}

void transmitPacket(char *d, int s){
	volatile unsigned int j;
	P2OUT |= Tx_Enable;
	UART_putChar(START_BYTE);
	UART_putChar('X');
	for (j = 0; j < s; ++j) {
		UART_putChar(d[j]);
	}
	UART_putChar(END_BYTE);
	P2OUT &= ~Tx_Enable;
}

char checkPacket(char *d){

	for (i = 1; i < 20; ++i) {
		if (d[i] != dataCheck[i]) {
			return 'A';
		}
	}
	return 'B';
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	while (!(IFG2&UCA0TXIFG));                	// USCI_A0 TX buffer ready?
	byteRX = UCA0RXBUF;                     	// Store RXed character

	uART_test[counterTemp] = byteRX;

	if(storeDataFlag == 1){

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
			myTimeSlot = 0;
			if (counterTemp >= 20) {
				if (checkPacket(uartBuffer) == 'A') {
					blinkLEDRed(2);

				}
				else if (checkPacket(uartBuffer) == 'B') {
					blinkLEDRednGreen(2);
					runCommRoutine();
				}
			}
		}

		else if (!pcktStartFlag && byteRX == '>') {
			storeDataInVar();
			storeDataFlag = 0;
		}

		counterTemp++;										//temporary counter used for testing only

		if (counterTemp > 38) {								//used for testing only
			counterTemp = 0;								//used for testing only
		}
	}
}
