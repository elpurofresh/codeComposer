

#include <msp430x16x.h>

/* Peripheral clock rate, in Hz, used for timing. */
#define PERIPH_CLOCKRATE 8000000
/* Number of times to retry the probe cycle during initialization */
#define SD_INIT_TRY 50

static unsigned char response[5];
static unsigned char argument[4];

/* Assert the CS signal, active low (CS=0) */
void spi_cs_assert()
{
	// Pin 2.0, Pin 8
	P2OUT &= ~0x010;
}

/* Deassert the CS signal (CS=1) */
void spi_cs_deassert()
{
	// Pin 2.0, Pin 8
	P2OUT |= 0x010;
}

/* Send a single byte over the SPI port */
void spi_send_byte(unsigned char input)
{
	IFG1 &= ~URXIFG0;
	/* Send the byte */
	TXBUF0 = input;
	/* Wait for the byte to be sent */
	while ((IFG1 & URXIFG0) == 0) { }
}

/* Receive a byte. Output an 0xFF (the bus idles high) to receive the byte */
unsigned char spi_rcv_byte()
{
	unsigned char tmp;
	IFG1 &= ~URXIFG0;
	/* Send the byte */
	TXBUF0 = 0xFF;
	/* Wait for the byte to be received */
	while ((IFG1 & URXIFG0) == 0) { }
	tmp = U0RXBUF;
	return (tmp);
}

void sd_delay(char number)
{
	char i;
	/* Null for now */
	for (i=0; i<number; i++)
	{
		/* Clock out an idle byte (0xFF) */
		spi_send_byte(0xFF);
	}
}

int sd_send_command(sd_context_t *sdc,
		unsigned char cmd, unsigned char response_type,
		unsigned char *response, unsigned char *argument)
{
	int i;
	char response_length;
	unsigned char tmp;
	spi_cs_assert();
	/* All data is sent MSB first, and MSb first */
	/* Send the header/command */
	/* Format:
	cmd[7:6] : 01
	cmd[5:0] : command */
	spi_send_byte((cmd & 0x3F) | 0x40);
	for (i=3; i>=0; i--)
	{
		spi_send_byte(argument[i]);
	}
	/* This is the CRC. It only matters what we put here for the first
	command. Otherwise, the CRC is ignored for SPI mode unless we
	enable CRC checking. */
	spi_send_byte(0x95);
	response_length = 0;
	switch (response_type)
	{
	case R1:
	case R1B:
		response_length = 1;
		break;
	case R2:
		response_length = 2;
		break;
	case R3:
		response_length = 5;
		break;
	default:
		break;
	}
	/* Wait for a response. A response can be recognized by the
		start bit (a zero) */
	i=0;
	do
	{
		tmp = spi_rcv_byte();
		i++;
	}
	while (((tmp & 0x80) != 0) && i < SD_CMD_TIMEOUT);
	/* Just bail if we never got a response */
	if (i >= SD_CMD_TIMEOUT)
	{
		spi_cs_deassert();
		return 0;
	}
	for (i=response_length-1; i>=0; i--)
	{
		response[i] = tmp;
		/* This handles the trailing-byte requirement. */
		tmp = spi_rcv_byte();
	}
	/* If the response is a "busy" type (R1B), then there's some
	 * special handling that needs to be done. The card will
	 * output a continuous stream of zeros, so the end of the BUSY
	 * state is signaled by any nonzero response. The bus idles
	 * high.
	 */
	i=0;
	if (response_type == R1B)
	{
		do
		{
			i++;
			tmp = spi_rcv_byte();
		}
		/* This should never time out, unless SDI is grounded.
		 * Don't bother forcing a timeout condition here. */
		while (tmp != 0xFF);
		spi_send_byte(0xFF);
	}
	spi_cs_deassert();
	return 1;
}

int sd_initialize()
{
	char i, j;
	sdc->busyflag = 0;
	for (i=0; i<4; i++)
		argument[i] = 0;
	/* Delay for at least 74 clock cycles. This means to actually
	 * *clock* out at least 74 clock cycles with no data present on
	 * the clock. In SPI mode, send at least 10 idle bytes (0xFF). */
	spi_cs_assert();
	sd_delay(100);
	spi_cs_deassert();
	sd_delay(2);
	/* Put the card in the idle state */
	if (sd_send_command(sdc, CMD0, CMD0_R, response, argument) == 0)
		return 0;
	/* Now wait until the card goes idle. Retry at most SD_IDLE_WAIT_MAX times */
	j = 0;
	do
	{
		j++;
		/* Flag the next command as an application-specific command */
		if (sd_send_command(sdc, CMD55, CMD55_R, response, argument) == 1)
		{
			/* Tell the card to send its OCR */
			sd_send_command(sdc, ACMD41, ACMD41_R, response, argument);
		}
		else
		{
			/* No response, bail early */
			j = SD_IDLE_WAIT_MAX;
		}
	}
	while ((response[0] & MSK_IDLE) == MSK_IDLE && j < SD_IDLE_WAIT_MAX);
	/* As long as we didn't hit the timeout, assume we're OK. */
	if (j >= SD_IDLE_WAIT_MAX)
		return 0;
	if (sd_send_command(sdc, CMD58, CMD58_R, response, argument) == 0)
		return 0;
	/* At a very minimum, we must allow 3.3V. */
	if ((response[2] & MSK_OCR_33) != MSK_OCR_33)
		return 0;
	/* Set the block length */
	if (sd_set_blocklen (sdc, SD_BLOCKSIZE) != 1)
		return 0;
	/* If we got this far, initialization was OK. */
	return 1;
}

int do_sd_initialize (void) {

	/* Initialize and enable the SPI module */
	P1SEL = 0x00E; // Setup P1 for SPI mode
	P1OUT |= 0x000; // Setup P3.4 as the SS signal, active low. So, initialize it high.
	P1DIR |= 0x000; // Set up P3.4 as an output
	U0CTL = (CHAR | SYNC | MM | SWRST); // 8-bit, SPI, Master
	U0TCTL = (SSEL1 | STC | CKPH); // Normal polarity, 3-wire
	U0BR0 = 0x002; // SPICLK = SMCLK/2 (2=Minimum divisor)
	U0BR1 = 0x000;
	U0MCTL = 0x000;
	ME1 |= USPIE0; // Module enable
	U0CTL &= ~SWRST; // SPI enable

	/* Set the baud-rate divisor. The correct value is computed by dividing
	the clock rate by the desired baud rate. The minimum divisor allowed
	is 2. */
	U0CTL |= SWRST; // Temporarily disable the SPI module
	U0BR1 = (PERIPH_CLOCKRATE/400000) >> 8;
	U0BR0 = (PERIPH_CLOCKRATE/400000);
	U0CTL &= ~SWRST; // Re-enable SPI

	/* Initialization OK? */
	if (sd_initialize() != 1)
		return 0;
	/* Set the maximum SPI clock rate possible */
	spi_set_divisor(2);
	return 1;

}

void main(void) {

	int i = 0, ok = 0;

	WDTCTL = WDTPW | WDTHOLD; /* Stop the watchdog timer */

	for (i=0; i<SD_INIT_TRY && ok != 1; i++)
	{
		ok = do_sd_initialize();
	}


}


