/*
 * RTC_variables.h
 *
 *  Created on: Jun 5, 2012
 *      Author: Greg Wells
 */

#ifndef RTC_VARIABLES_H_
#define RTC_VARIABLES_H_

#define CTRLZ 		0x1A 			// Refer to SUB = Ctrl+Z in ASCII Matrix
#define CR			0xD				// Refer to CR = ENTER in ASCII Matrix
#define Red_LED		BIT0
#define Green_LED	BIT6
#define MasterNote	BIT4			// Notifys the master when a collection or transmission needs to be made
#define UART_RXD	BIT1			// RXD on P1.1
#define UART_TXD	BIT2        	// TXD on P1.2
#define Bot_ID		0				//this is bot 0
#define net_size	4				//the network contains 4 bots
#define ON  		1
#define OFF			0

#endif /* RTC_VARIABLES_H_ */
