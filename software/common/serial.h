/******************************************************************************/
/* File    :	serial.h						      */
/* Function:	Include file of 'serial.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef SERIAL_H
#define SERIAL_H

#include "../common/app_prefs.h"

#ifndef HAS_SERIAL

/* Serial support DISABLED */

#define TX(a)
#define TX2(a,b)
#define TX3(a,b,c)
#define TX4(a,b,c,d)
#define TX5(a,b,c,d,e)

#else // HAS_SERIAL

/* Serial support ENABLED */

#define TX(a)			printf(a)
#define TX2(a,b)		printf(a,b)
#define TX3(a,b,c)		printf(a,b,c)
#define TX4(a,b,c,d)	printf(a,b,c,d)
#define TX5(a,b,c,d,e)	printf(a,b,c,d,e)

#ifdef SERIAL_C
#	include "../common/prot_src.h"
#else
#	include "../common/prot_inc.h"
#endif

#define XON				0x11	/* ASCII value for Xon (^S) */
#define XOFF			0x13	/* ASCII value for Xoff (^Q) */

// Serial flow control options
#define SERIAL_FLOW_NONE		0x00
#define SERIAL_FLOW_XONXOFF 	0x01

#define SERIAL_LINETERM_CR		0
#define SERIAL_LINETERM_LF		1
#define SERIAL_LINETERM_CRLF	2

typedef struct {
	unsigned initialized		: 1;
	unsigned lineterm			: 2;
} serial_state_t;

#ifdef SERIAL_C
serial_state_t serial_state = {
	0,
#ifdef SERIAL_LINETERM_CRLF_FORCE
	SERIAL_LINETERM_CRLF
#elif defined SERIAL_LINETERM_LF_FORCE
	SERIAL_LINETERM_LF
#else
	SERIAL_LINETERM_CR
#endif
};
#else
extern serial_state_t serial_state;
#endif

void		serial_init	(unsigned long	bitrate,
				 unsigned char	flow);
void		serial_term	(void);
void		serial_rx_isr	(void);
void		serial_tx_isr	(void);
void		putch		(unsigned char	c);
unsigned char	readch		(char		*ch);
unsigned char	serial_wait_s	(const char	*s,
				 unsigned long	timeout);

#endif /* HAS_SERIAL */

#endif /* SERIAL_H */
