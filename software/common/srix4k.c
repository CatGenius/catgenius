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

static struct timer	timer    = EXIRED;
static unsigned char	state = 0;


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
		settimeout(&timer, SECOND);
		state++;
	}

	if (cr14_busy())
		return;

	switch (state) {
	case IDLE:
		break;
	case CARRIEROFF:
		cr14_writeparamreg(0x00);
		state++;
		break;
	case CARRIEROFF+1:
		state = IDLE;
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