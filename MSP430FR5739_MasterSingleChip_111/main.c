/*
 * main.c
 */

#include "msp430fr5739.h"
#include "common_functions.h"
#include "common_variables.h"

void txPacket(char *buffer);
void broadCast(int val);

void storeSdCard(void);
void savingToSdCard(void);

void resetClock(void);
void timeKeeper(void);

void networkProtocol(void);

void onSequence(void);

volatile int initCorrect = 0;
volatile int cnterTest = 0;
char uart_Test[60] =  "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
volatile int successPcktRxD = FALSE;
volatile int counter = 0;


void main(void) {

	initClocks();
	initUARTSdCard();
	initUARTBot();

	setupPorts();
	onSequence();

	_EINT();

	initCorrect = 1;
	myTimeSlotMax = 2*(numNodes*numNodes)-1;

	startExternalClock();

	while(initCorrect){

		timeKeeper();

		if (successPcktRxD == TRUE) {
			parsePacket(uartBuffer);
			//storeSdCard();
			networkProtocol();
			successPcktRxD = FALSE;
			counter++;
		} else {
			startRXBots();
			__low_power_mode_3();
		}
	}
}

void networkProtocol(void)
{
	while (myTimeSlotCounter < myTimeSlotMax || myParent == NLL) {

		while( (sec % timePerInterval) != 0 ) { }; //myTimeSlotInterval = sec % timePerInterval;

		if ((myTimeSlotCounter % numNodes != myBotID) || myParent == NLL) {

			if (successPcktRxD == TRUE) {
				parsePacket(uartBuffer);
				storeSdCard();
				successPcktRxD = FALSE;

				/*if (isChildrenSubTreeDone() == TRUE) {
					mySubTreeDone = 1;
					sendOnce = 1;
				}*/

				if (myParent == NLL) {
					myParent = packetBotId;
					myTimeSlotCounter = packetTimeSlot;

				} else if (packetParent == myBotID) {
					putBotMyChildrenList(packetBotId);
					iHaveChildren = TRUE;

					/*if (packetSubTree == TRUE) {
						setDoneFinishChildSubTreeList(packetBotId);
					}*/

				}
				myTimeSlotCounter = packetTimeSlot;
			}

		} else {

			if (myParent != NLL) {// && sendOnce <= 1) {
				getMyData();
				broadCast(SELF_NETWORK_DATA);

				if (mySndFstTime == -1) {
					mySndFstTime = myTimeSlotCounter;
				}
				//sendOnce = 2;
			}
		}

		if ((myTimeSlotCounter == (mySndFstTime + numNodes)) && iHaveChildren == FALSE) {
			iAmALeaf = TRUE;
		}

		if ((iAmALeaf == TRUE || myChildrenSntDataSuccess == TRUE) &&
				(myTimeSlotCounter % numNodes) == myBotID) {
				//&& sendOnce > 1) {
			getMyData();
			broadCast(SELF_N_CHILD_DATA);

		} else {
			if (successPcktRxD == TRUE) {
				//save to RAM
				parsePacket(uartBuffer);
				storeSdCard();
				successPcktRxD = FALSE;

				setRxdFromChildren(packetBotId);

				if (rxdFromAllChildrenTest() == TRUE) {
					myChildrenSntDataSuccess = TRUE;
				}
			}
		}
		myTimeSlotCounter++; //myTimeSlotCounter++;
	}
	//myTimeSlotCounter = 0;
}

void onSequence(void)
{
	P1OUT |= BIT0;				//Red LED
	_delay_cycles(2000000);
	P1OUT &= ~BIT0;
	P1OUT |= BIT1;				//Yellow LED
	_delay_cycles(2000000);
	P1OUT &= ~BIT1;
}

void txPacket(char *buffer)
{
	//static int i = 0;
	static int j = 0;

	//for (i = 0; i < 3; ++i) {
	UART_BotputChar(START_BYTE);

	for (j = 0; j < strlen(buffer); ++j) {  // Send 5 times the data packet with a 0.25sec pause in between
		UART_BotputChar(buffer[j]);
	}
	UART_BotputChar(END_BYTE);

	//sleepQuart();//<--must revise the need for this delay (prolly intended for the SD card)
	//}
}

void broadCast(int val)
{
	genPacket(val);
	txPacket(buffToNetwork);
}

void storeSdCard(void)
{
	stopRXBots();
	stopExternalClock();
	savingToSdCard();
	startExternalClock();
}

void savingToSdCard(void)			// Saves received data on to SD card and operates on the packet
{
	turnSdCard(ON);
	writeDataToSDFile("append myData.txt\0", uartBuffer, 1);
	turnSdCard(OFF);
}

void resetClock(void)
{
	stopExternalClock();

	char temp[3] = "00\0";
	temp[0] = uartBuffer[6];
	temp[1] = uartBuffer[7];
	hrs = atoi(temp);

	temp[0] = uartBuffer[8];
	temp[1] = uartBuffer[9];
	min = atoi(temp);

	temp[0] = uartBuffer[10];
	temp[1] = uartBuffer[11];
	sec = atoi(temp);

	startExternalClock();
}

void timeKeeper(void)
{
	if (sec == 60) { sec = 0; min ++; }
	if (min == 60) { min = 0; hrs++;  }
	if (hrs == 24) { hrs = 0; 		  }
}



// Receives data from other robots
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,0x02))	  // Tells the compiler that UCAxIV can take only even values up to 0x08 to make a more efficient jump table
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		uart_Test[cnterTest++] = UCA0RXBUF;

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
			successPcktRxD = TRUE;

		}

		if (cnterTest > 60) {
			cnterTest = 0;
		}

		//stopRXBots();

		break;

	default: break;
	}

	__low_power_mode_off_on_exit();
}

// Receives data from the SD card
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	switch(__even_in_range(UCA1IV,0x02))
	{
	case 0:break;                             // Vector 0 - no interrupt
	case 2:                                   // Vector 2 - RXIFG

		byteRX_SD = UCA1RXBUF;                     // Store RXed character

		//uart_Test[cnterTest] = byteRX_SD;

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

		/*cnterTest++;
		if (cnterTest > 60) {
			cnterTest = 0;
		}*/

		break;

	default: break;
	}

	__low_power_mode_off_on_exit();
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1OUT ^= BIT2; //Green LED debugging?

	sec++;
	P1IFG &= ~BIT4;				// Clear P1.4 IFG

	__low_power_mode_off_on_exit();
}


