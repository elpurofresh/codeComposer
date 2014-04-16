/**
 * main.c - Launchpad compatible full-duplex software UART example program.
 *
 * SoftSerial:
 * Author: Rick Kimball
 * email: rick@kimballsoftware.com
 * Version: 1.00 Initial version 04-20-2011
 * Version: 1.01 cleanup 04-21-2011
 *
 */

#include <msp430.h>
#include <stdarg.h>
#include "config.h"
#include "softserial.h"
#include "pff2a/src/diskio.h"
#include "pff2a/src/pff.h"
#include "print/print.h"
#include "drivers/spi.h"



char Line[20];

static
void put_rc (FRESULT rc)
{
	const char *p;
	static const char str[] =
		"OK\0" "DISK_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"NOT_OPENED\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && *p; i++) {
		while(*p++) ;
	}
	printf("rc=%u FR_%s\n", (long int)rc, p);
}
#if 1
static
void put_dump (
	const void* buff,		/* Pointer to the byte array to be dumped */
	unsigned long addr,		/* Heading address value */
	int len					/* Number of bytes to be dumped */
)
{
	int i;
	const unsigned char *p = buff;

#if 0
	printf("%8lX ", addr);		/* address */


	for (i = 0; i < len; i++)		/* data (hexdecimal) */
		printf(" %2X", p[i]);
#endif
	SoftSerial_xmit(' ');
	for (i = 0; i < len; i++)		/* data (ascii) */
		SoftSerial_xmit((p[i] >= ' ' && p[i] <= '~') ? p[i] : '.');

	SoftSerial_xmit('\n');
}
#endif
/**
 * setup() - initialize timers and clocks
 */

void setup() {
    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer

    DCOCTL = 0x00;                  // Set DCOCLK to 16MHz
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    SoftSerial_init();              // Configure TIMERA

    /**
     * Setting these flags allows you to easily measure the actual SMCLK and ACLK
     * frequencies using a multimeter or oscilloscope.  You should verify that SMCLK
     * and your desired F_CPU are the same.
     */

    P1DIR |= BIT0; P1SEL |= BIT0;   // measure P1.0 for actual ACLK
    P1DIR |= BIT4; P1SEL |= BIT4;   // measure P1.4 for actual SMCLK

    _enable_interrupts(); // let the timers do their work
}

/**
 * loop() - this routine runs over and over
 *
 * Wait for data to arrive. When something is available,
 * read it from the ring_buffer and echo it back to the
 * sender.
 */

void loop() {
    int c;

    if ( !SoftSerial_empty() ) {
        while((c=SoftSerial_read()) != -1) {
            SoftSerial_xmit((uint8_t)c);
        }
    }
}

static void put_drc(long int res)
{
	printf("rc=%d\n", res);
}

static void get_line(char *buff, BYTE len)
{
	BYTE c, i;

	i = 0;
	for (;;) {
		if ( !SoftSerial_empty() ) 
		{
			c=SoftSerial_read();
		if (c == '\r') break;
		if ((c == '\b') && i) i--;
		if ((c >= ' ') && (i < len - 1))
				buff[i++] = c;			
		}

	}
	buff[i] = 0;
	//xmit('\n');
}


/**
 * main - sample program main loop
 *
 */
void main (void)
{
	char *ptr;
	long p1, p2;
	long int res;//BYTE res;
	WORD s1, s2, s3, ofs, cnt, w;
	FATFS fs;			/* File system object */
	DIR dir;			/* Directory object */
	FILINFO fno;		/* File information */
	
	long int i = 22;


	setup();
	spi_initialize();

	printf("\nPFF test monitor\n");
    
		
	for (;;)
	{
		printf(">");
		get_line(Line, sizeof(Line));
		ptr = Line;
		switch (*ptr++)
		{
			case 'd' :
				switch (*ptr++)
				{
					case 'i' :	/* di - Initialize physical drive */
					printf("Disk Initialize %d \n",i);	
					res = disk_initialize();
					put_drc(res);
					break;
				}
				break;
			case 'f' :		
				switch (*ptr++)
				{
					case 'i' :	/* fi - Mount the volume */
						put_rc(pf_mount(&fs));
						break;
					case 'o' :	/* fo <file> - Open a file */
						while (*ptr == ' ') ptr++;
						put_rc(pf_open(ptr));
						break;
					#if _USE_READ
					case 'd' :	/* fd - Read the file 128 bytes and dump it */
						ofs = fs.fptr;
						res = pf_read(Line, sizeof(Line), &s1);
						if (res != FR_OK) { put_rc(res); break; }
						ptr = Line;
						while (s1) {
							s2 = (s1 >= 16) ? 16 : s1;
							s1 -= s2;
							put_dump((BYTE*)ptr, ofs, s2);
							ptr += 16; ofs += 16;
						}
						break;
		
					case 't' :	/* ft - Type the file data via dreadp function */
						do {
							res = pf_read(0, 32768, &s1);
							if (res != FR_OK) { put_rc(res); break; }
						} while (s1 == 32768);
						break;
					#endif						
					#if _USE_DIR
					case 'l' :	/* fl [<path>] - Directory listing */
						while (*ptr == ' ') ptr++;
						res = pf_opendir(&dir, ptr);
						if (res) { put_rc(res); break; }
						s1 = 0;
						for(;;) {
							res = pf_readdir(&dir, &fno);
							if (res != FR_OK) { put_rc(res); break; }
							if (!fno.fname[0]) break;
							if (fno.fattrib & AM_DIR)
								printf("   <DIR>   %s\n", fno.fname);
							else
								printf("%9u  %s\n", fno.fsize, fno.fname);
							s1++;
						}
						printf("%u item(s)\n", s1);
						break;
					#endif		
				}	
		}	

	}
    /*while( 1 ) {
        loop();
    }*/
}


