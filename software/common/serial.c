#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "serial.h"

#define RXBUFFER			/* Use buffers for received characters */
//#define TXBUFFER			/* Buffers for transmitted character not implemented yet */
#define XONXOFF				/* Use Xon/Xoff handshaking (for receiving only) */
#define BUFFER_SIZE		8	/* Must to be a power of 2 or roll-overs need to be taken into account */
#define BUFFER_SPARE		3	/* Minumum number of free positions before issuing Xoff */

#define INTDIV(t,n)		((2*(t)+(n))/(2*(n)))		/* Macro for integer division with proper round-off (BEWARE OF OVERFLOW!) */
#define FREE(h,t,s)		(((h)>=(t))?((s)-((h)-(t))-1):((t)-(h))-1)

#ifdef XONXOFF
#define XON			0x11
#define XOFF			0x13
#endif /* XONXOFF */


struct queue {
	char		buffer[BUFFER_SIZE];
	unsigned	head	: 3;
	unsigned	tail	: 3;
	unsigned	full	: 1;
};

#ifdef RXBUFFER
struct queue		rx;
#endif /* RXBUFFER */
#ifdef TXBUFFER
struct queue		tx;
#endif /* TXBUFFER */


void serial_init(unsigned long bitrate)
{
#ifdef RXBUFFER
	rx.head = 0;
	rx.tail = 0;
#endif /* RXBUFFER */
#ifdef TXBUFFER
	tx.head = 0;
	tx.tail = 0;
#endif /* TXBUFFER */

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
#ifdef RXBUFFER
	RCIE = 1;	/* Enable rx interrupts */
#else /* RXBUFFER */
	RCIE = 0;	/* Disable rx interrupts */
#endif /* RXBUFFER */
#ifdef TXBUFFER
	TXIE = 1;	/* Enable tx interrupts */
#else /* TXBUFFER */
	TXIE = 0;	/* Disable tx interrupts */
#endif /* TXBUFFER */
	RX9  = 0;	/* 8-bit reception mode */
	TX9  = 0;	/* 8-bit transmission mode */
	CREN = 0;	/* Reset receiver */
	CREN = 1;	/* Enable reception */
	TXEN = 0;	/* Reset transmitter */
	TXEN = 1;	/* Enable transmission */

#ifdef XONXOFF
	TXREG = XON;
#endif /* XONXOFF */
}


void serial_term(void)
{
#ifdef RXBUFFER
	RCIE = 0;	/* Disable rx interrupts */
#endif /* RXBUFFER */
#ifdef TXBUFFER
	while (tx.head != tx.tail);
	TXIE = 0;	/* Disable tx interrupts */
#endif /* TXBUFFER */

	while(!TXIF);	/* Wait for the last character to go */
#ifdef XONXOFF
	TXREG = XOFF;
	while(!TXIF);
#endif /* XONXOFF */

	CREN = 0;	/* Disable reception */
	TXEN = 0;	/* Disable transmission */
}


void serial_rx_isr(void)
{
#ifdef RXBUFFER
	/* Handle overflow errors */
	if (OERR) {
		rx.buffer[rx.head] = RCREG; /* Read RX register, but do not queue */
		TXEN = 0;
		TXEN = 1;
		CREN = 0;
		CREN = 1;
		return;
	}
	/* Handle framing errors */
	if (FERR) {
		rx.buffer[rx.head] = RCREG; /* Read RX register, but do not queue */
		TXEN = 0;
		TXEN = 1;
		return;
	}
	/* Read RX register */
	rx.buffer[rx.head] = RCREG;
	/* Add read character to the queue */
	rx.head++;
#ifdef XONXOFF
	if (FREE(rx.head, rx.tail, BUFFER_SIZE) <= (BUFFER_SPARE)) {
		while(!TXIF);	// TBD: Could potentially wait forever here
		TXREG = XOFF;
	}
#endif /* XONXOFF */
	/* Check for an overflow */
	if (rx.head == rx.tail)
		/* Dequeue the oldest character */
		rx.tail++;
#endif /* RXBUFFER */
}


void serial_tx_isr(void)
{
#ifdef TXBUFFER
#endif /* TXBUFFER */
}


/* Write a character to the serial port */
void putch(char ch)
{
#ifdef TXBUFFER
#else
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
	// TBD: Delay based on baud rate?
#endif /* TXBUFFER */
}


/* Read a character from the serial port */
unsigned char readch(char *ch)
{
	unsigned char	result = 0;

#ifdef RXBUFFER
	RCIE = 0;	/* Disable rx interrupts */
#ifdef TXBUFFER
	TXIE = 0;	/* Disable tx interrupts */
#endif /* TXBUFFER */

	/* Check if there's anything to read */
	if (rx.head != rx.tail) {
		/* Copy the character */
		*ch = rx.buffer[rx.tail];
		/* Dequeue the character */
		rx.tail++;
#ifdef XONXOFF
		/* Check if this was the last character */
		if (rx.head == rx.tail) {
			while(!TXIF);
			TXREG = XON;
		}
#endif /* XONXOFF */
		result = 1;
	}

	RCIE = 1;	/* Enable rx interrupts */
#ifdef TXBUFFER
	TXIE = 1;	/* Enable tx interrupts */
#endif /* TXBUFFER */
#else /* !RXBUFFER */
	if (!RCIF)
		goto out;

	if (OERR) {
		TXEN = 0;
		TXEN = 1;
		CREN = 0;
		CREN = 1;
		goto out;
	}
	if (FERR) {
		*ch  = RCREG;
		TXEN = 0;
		TXEN = 1;
		goto out;
	}

	*ch  = RCREG;
	result = 1;
out:
#endif /* RXBUFFER */
	return result;
}

