/*
 * main.c
 */

#include "msp430g2553.h"
#include <string.h>


/*
 * To avoid incrementing the size of the array that holds
 * the seconds, mins and hrs chars values and that will
 * be converted to integers, this constant is defined.
 * itoa and atoi functions have been modified accordingly,
 * in particular atoi stops its conversion on this maximum
 * value and not on a char outside the range 0-9.
 */
#define MAX_NUM_DIGITS 2

extern union // <<<<<< notice I omitted "volatile", in order to optimize code done on this register
{
    unsigned char value;

    struct
    {
    unsigned char botIDX	: 1;  // Watchdog Interrupt Enable
    unsigned char botID0   	: 1;  // Osc. Fault  Interrupt Ena
    unsigned char botID1   	: 1;
    unsigned char botID2   	: 1;  // NMI Interrupt Enable
    unsigned char parent  	: 1;  // Flash Access Violation Interrupt Enable
    unsigned char hasChd   	: 1;
    unsigned char isLeaf   	: 1;
    unsigned char sndFst	: 1;
    }headerDataBits;

} MYIE1;

void itoa(int value, char s[]);
int atoi(const char *s);

volatile int sec = 5;
volatile int min = 42;
volatile int hrs = 18;

volatile int tsec = 0;
volatile int tmin = 0;
volatile int thrs = 0;

/*
 * headerData contains the:
 * 1. botID
 * 2. Parent (yes/no)
 * 3. hasChildren (yes/no)
 * 4. isLeaf (yes/no)
 * 5. sendFirstTime (yes/no)
 *
 */
volatile int headerData[5] = {4, 0, 1, 1, 0};
volatile int tempData[5] = {9, 9, 9, 9, 9};
volatile char buffer[11] = "@@@@@@@@@@@";

char temp[3] = "00\0";  //'\0' is a null char representing the end of the string

volatile int negValue = -1;

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;            // Set DCO
	DCOCTL = CALDCO_1MHZ;

	itoa(sec, temp);
	tsec = atoi(temp);
	sec = tsec + 5;

	itoa(min, temp);
	tmin = atoi(temp);
	min = tmin + 10;

	itoa(hrs, temp);
	thrs = atoi(temp);
	hrs = thrs + 15;

	itoa(negValue, temp);
	hrs = atoi(temp);
	negValue = hrs + 2;

}

/*
 * Extremely simplified version of itoa.
 * Size of string is NOT fixed and unknown, it does include
 * the last char in a string '\0' (the null char). It does
 * checks for a sign.
 * Finally, the function already puts the values of the integer
 * in the correct order, no need to reverse them.
 */
void itoa(int value, char s[])
{
	int i = strlen(s); // MAX_NUM_DIGITS = 2
	int sign = 0;

	if ((sign = value) < 0 ) {
		value = -value;
	}

	do {
		s[--i] = value % 10 + '0';

	} while ( (value /= 10) > 0);

	if (sign < 0) {
		s[0] = '-';
	}
}

/*int atoi(char s[])
{
	int i = 0, n = 0;

	for (n = 0; s[i] >= '0' && s[i] <= '9' && i < 2; i++) {
		n = 10 * n + (s[i] - '0');
	}

	return n;
}*/

/*
 * Extremely simplified version of atoi.
 * Size of string is NOT fixed and unknown, it does check for
 * the last char in a string '\0' (the null char) but not for
 * a sign, it assumes values are always positive.
 */
int atoi(const char *s)
{
	int i= 0, n = 0, sign = 0;

	sign = (*s == '-') ? -1 : 1;

	if (*s == '-') {
		s++;
	}

	for (n = 0; *s >= '0' && *s <= '9'; s++/*, i++*/) {
		n = 10 * n + (*s - '0');
	}

	return sign*n;
}








