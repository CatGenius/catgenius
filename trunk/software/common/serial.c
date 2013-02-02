#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */


#define INTDIV(t,n)		((2*(t)+(n))/(2*(n)))		/* Macro for integer division with proper round-off (BEWARE OF OVERFLOW!) */


void serial_init(unsigned long bitrate)
{
#if (defined _16F877A) || (defined _16F887)
	SPBRG = INTDIV(INTDIV(_XTAL_FREQ, bitrate), 16UL)-1;
#elif (defined _16F1939)
	BRG16 = 1;	/* Use 16-bit bit rate generation */
	if (BRG16) {
		/* 16-bit bit rate generation */
		unsigned long	divider = INTDIV(INTDIV(_XTAL_FREQ, 4UL), bitrate)-1;

		SPBRG  =  divider & 0x00FF;
		SPBRGH = (divider & 0xFF00) >> 8;
	} else
		/* 8-bit bit rate generation */
		SPBRG = INTDIV(INTDIV(_XTAL_FREQ, bitrate), 16UL)-1;
#endif /* _16F877A/_16F887/_16F1939 */

	CSRC = 1;	/* Clock source from BRG */
	BRGH = 1;	/* High-speed bit rate generation */
	SYNC = 0;	/* Asynchronous mode */
	SPEN = 1;	/* Enable serial port pins */
	TXIE = 0;	/* Disable tx interrupts */
	RCIE = 0;	/* Disable rx interrupts */
	TX9  = 0;	/* 8-bit transmission mode */
	RX9  = 0;	/* 8-bit reception mode */
	CREN = 0;	/* Reset receiver */
	CREN = 1;	/* Enable reception */
	TXEN = 0;	/* Reset transmitter */
	TXEN = 1;	/* Enable transmission */
}


void serial_term(void)
{
	CREN = 0;	/* Disable reception */
	TXEN = 0;	/* Disable transmission */
}


/* Write a character to the serial port */
void putch(char ch)
{
	if (!SPEN)
		return;
	while(!TXIF) {	/* Wait for TXREG to be empty */
		if (OERR) {
			TXEN = 0;
			TXEN = 1;
			CREN = 0;
			CREN = 1;
		}
		if (FERR) {
			volatile unsigned char dummy;

			dummy = RCREG;
			TXEN  = 0;
			TXEN  = 1;
		}
		CLRWDT();
	}
	TXREG = ch;
	CLRWDT();
}


/* Read a character from the serial port */
unsigned char readch(char *ch)
{
	if (!RCIF)
		return 0;

	if (OERR) {
		TXEN = 0;
		TXEN = 1;
		CREN = 0;
		CREN = 1;
		return 0;
	}
	if (FERR) {
		*ch  = RCREG;
		TXEN = 0;
		TXEN = 1;
		return 0;
	}

	*ch  = RCREG;
	return 1;
}
