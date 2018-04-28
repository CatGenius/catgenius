#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "serial.h"


#define RXBUFFER			/* Use buffers for received characters */
//#define TXBUFFER			/* Use buffers for transmitted character (MAKE SURE TO ENABLE INTERRUPTS BEFORE TRANSMITTING ANYTHING) */
#define BUFFER_SIZE		8	/* Must to be a power of 2 or roll-overs need to be taken into account */
#define BUFFER_SPARE		2	/* Minumum number of free positions before issuing Xoff */

#define INTDIV(t,n)		((2*(t)+(n))/(2*(n)))		/* Macro for integer division with proper round-off (BEWARE OF OVERFLOW!) */
#define FREE(h,t,s)		(((h)>=(t))?((s)-((h)-(t))-1):((t)-(h))-1)

#define XON			0x11	/* ASCII value for Xon (^S) */
#define XOFF			0x13	/* ASCII value for Xoff (^Q) */


struct queue {
	char		buffer[BUFFER_SIZE];	/* Here's where the data goes */
	unsigned	head		: 3;	/* Index to a currently free position in buffer */
	unsigned	tail		: 3;	/* Index to the oldes occupied position in buffer, if not equal to head */
	unsigned	xon_enabled	: 1;	/* Specifies if Xon/Xoff should be issued/adhered to */
	unsigned	xon_state	: 1;	/* Keeps track of current Xon/Xoff state for this queue */
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
	rx.head        = 0;
	rx.tail        = 0;
	rx.xon_enabled = 1;
	rx.xon_state   = 1;
#endif /* RXBUFFER */

#ifdef TXBUFFER
	tx.head        = 0;
	tx.tail        = 0;
	tx.xon_enabled = 1;
	tx.xon_state   = 1;
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
	RCIE = 0;	/* Disable rx interrupt */
	TXIE = 0;	/* Disable tx interrupt */
	RX9  = 0;	/* 8-bit reception mode */
	TX9  = 0;	/* 8-bit transmission mode */
	CREN = 0;	/* Reset receiver */
	CREN = 1;	/* Enable reception */
	TXEN = 0;	/* Reset transmitter */
	TXEN = 1;	/* Enable transmission */

#ifdef RXBUFFER
	RCIE = 1;	/* Enable rx interrupt */

	if (rx.xon_enabled) {
		TXREG = XON;
		rx.xon_state = 1;
	}
#endif /* RXBUFFER */
}


void serial_term(void)
{
#ifdef RXBUFFER
	RCIE = 0;	/* Disable rx interrupt */
#endif /* RXBUFFER */
#ifdef TXBUFFER
	while (tx.head != tx.tail);
	TXIE = 0;	/* Disable tx interrupt */
#endif /* TXBUFFER */

#ifdef RXBUFFER
	if (rx.xon_enabled && rx.xon_state) {
		while(!TXIF);
		TXREG = XOFF;
		rx.xon_state = 0;
	}
#endif /* RXBUFFER */
	while(!TXIF);	/* Wait for the last character to go */

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
	/* Copy the character from RX register into RX queue */
	rx.buffer[rx.head] = RCREG;
#ifdef TXBUFFER
	/* Check if an Xon or Xoff needs to be handled */
	if (tx.xon_enabled) {
		if (tx.xon_state && (rx.buffer[rx.head] == XOFF)) {
			tx.xon_state = 0;
			TXIE = 0;	/* Disable tx interrupt to stop transmitting */
			return;
		} else if (!tx.xon_state && (rx.buffer[rx.head] == XON)) {
			tx.xon_state = 1;
			/* Enable tx interrupt if tx queue is not empty */
			if (tx.head != tx.tail)
				TXIE = 1;
			return;
		}
	}
#endif /* TXBUFFER */
	/* Queue the character */
	rx.head++;
	/* Check if an Xoff is in required */
	if (rx.xon_enabled &&
	    rx.xon_state &&
	    (FREE(rx.head, rx.tail, BUFFER_SIZE) <= (BUFFER_SPARE))) {
		while(!TXIF);	// TBD: Could potentially wait forever here
		TXREG = XOFF;
		rx.xon_state = 0;
	}
	/* Check for an overflow */
	if (rx.head == rx.tail)
		/* Dequeue the oldest character */
		rx.tail++;
#endif /* RXBUFFER */
}


void serial_tx_isr(void)
{
#ifdef TXBUFFER
	/* Copy the character from the TX queue into the TX register */
	TXREG = tx.buffer[tx.tail];
	/* Dequeue the character */
	tx.tail++;

	/* Disable tx interrupt if queue is empty */
	if (tx.head == tx.tail)
		TXIE = 0;
#endif /* TXBUFFER */
}


/* Write a character to the serial port */
void putch(char ch)
{
#ifdef TXBUFFER
	unsigned char	queued = 0;

	do {
#ifdef RXBUFFER
		RCIE = 0;	/* Disable rx interrupt for concurrency */
#endif /* RXBUFFER */
		TXIE = 0;	/* Disable tx interrupt for concurrency */

		/* Check if there's room in the queue ((tx.head + 1) != tx.tail won't work due to bitfields being cast) */
		if (FREE(tx.head, tx.tail, BUFFER_SIZE)) {
			/* Copy the character into the TX queue */
			tx.buffer[tx.head] = ch;
			/* Queue the character */
			tx.head++;
			queued = 1;
		} else
			CLRWDT();
		if (!tx.xon_enabled || tx.xon_state)
			TXIE = 1;	/* Re-enable tx interrupt */
#ifdef RXBUFFER
		RCIE = 1;	/* Re-enable rx interrupt */
#endif /* RXBUFFER */
	} while (!queued);
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
#endif /* TXBUFFER */
}


/* Read a character from the serial port */
unsigned char readch(char *ch)
{
#ifdef RXBUFFER
	unsigned char	result = 0;

	RCIE = 0;	/* Disable rx interrupt for concurrency */
#ifdef TXBUFFER
	TXIE = 0;	/* Disable tx interrupt for concurrency */
#endif /* TXBUFFER */

	/* Check if there's anything to read */
	if (rx.head != rx.tail) {
		/* Copy the character from the RX queue */
		*ch = rx.buffer[rx.tail];
		/* Dequeue the character */
		rx.tail++;
		/* Check if an Xon is in required */
		if (rx.xon_enabled &&
		    !rx.xon_state &&
		    (rx.head == rx.tail)) {
			while(!TXIF);
			TXREG = XON;
			rx.xon_state = 1;
		}
		result = 1;
	}

	RCIE = 1;	/* Re-enable rx interrupt */
#ifdef TXBUFFER
	if ((tx.head != tx.tail) && (!tx.xon_enabled || tx.xon_state))
		TXIE = 1;	/* Re-enable tx interrupt */
#endif /* TXBUFFER */

	return result;

#else /* !RXBUFFER */
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
#endif /* RXBUFFER */
}

