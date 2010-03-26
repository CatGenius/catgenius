/******************************************************************************/
/* File    :	srix4k.c						      */
/* Function:	srix4k RFID tag functional implementation		      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	7 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "srix4k.h"
#include "hardware.h"
#include "cr14.h"

#include "timer.h"
#include "serial.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define IDLE		0
#define CARRIEROFF	1
#define CARRIERON	7
#define READUID
#define READBLOCK	12
#define WRITEBLOCK	22


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	timer    = EXPIRED;
static unsigned char	state = 0;

static unsigned char	frame[8];
static unsigned char	block = 0;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void srix4k_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: srix4k_init */


void srix4k_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		-       */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if( (state == IDLE) &&
	    timeoutexpired(&timer) ){
		state++;
	}

	if (cr14_busy())
		return;

	switch (state) {
	case IDLE:
		break;
	case 1:
		/* Turn on the carrier */
		cr14_writeparamreg(0x10);
		state++;
		break;
	case 2:
		/* Send an Init frame */
		frame[0] = 0x06;
		frame[1] = 0x00;
		cr14_writeframe(frame, 2);
		state++;
		break;
	case 3:
		/* Read the response */
		cr14_readframe();
		state++;
		break;
	case 4:
		/* Fetch the response */
		cr14_getframe(frame);
		state++;
	case 5:
		/* Select the chip */
		frame[1] = frame[0];
		frame[0] = 0x0E;
		cr14_writeframe(frame, 2);
		state++;
		break;
	case 6:
		/* Read the response */
		cr14_readframe();
		state++;
		break;
	case 7:
		/* Fetch the response */
		cr14_getframe(frame);
		state++;
	case 8:
		/* Get the unique ID */
putchhex(TRISC);
putch('\r');
putch('\n');
		frame[0] = 0x0B;
		cr14_writeframe(frame, 1);
		state++;
		break;
	case 9:
		/* Read the response */
		cr14_readframe();
		state++;
		break;
	case 10:
		/* Fetch the response */
		cr14_getframe(frame);
		putst("Unique ID: ");
		putchhex(frame[7]);
		putch(' ');
		putchhex(frame[6]);
		putch(' ');
		putchhex(frame[5]);
		putch(' ');
		putchhex(frame[4]);
		putch(' ');
		putchhex(frame[3]);
		putch(' ');
		putchhex(frame[2]);
		putch(' ');
		putchhex(frame[1]);
		putch(' ');
		putchhex(frame[0]);
		putch('\r');
		putch('\n');
		block = 0;
		state++;
	case 11:
		/* Get the unique ID */
		frame[0] = 0x08;
		frame[1] = block;
		cr14_writeframe(frame, 2);
		state++;
		break;
	case 12:
		/* Read the response */
		cr14_readframe();
		state++;
		break;
	case 13:
		/* Fetch the response */
		cr14_getframe(frame);
		putst("Block ");
		putchhex(block);
		putch(' ');
		putch(' ');
		putchhex(frame[3]);
		putch(' ');
		putchhex(frame[2]);
		putch(' ');
		putchhex(frame[1]);
		putch(' ');
		putchhex(frame[0]);
		putch(' ');
		putch('\r');
		putch('\n');
		block++;
		if (block > 127)
			block = 255;
		if (block)
			state = 11;
		else
			state++;
		break;
	case 14:
		/* Turn off the carrier */
		cr14_writeparamreg(0x00);
		state = IDLE;
		settimeout(&timer, 3*SECOND);
		break;
	}

} /* srix4k_work */


void srix4k_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	5 Mar 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: srix4k_term */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/