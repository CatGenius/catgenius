/******************************************************************************/
/* File    :	userinterface.c						      */
/* Function:	CatGenie user interface					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "userinterface.h"
#include "../common/timer.h"
#include "../common/catgenie120.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define HOLDTIME		(3 * SECOND)


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	holdtimeout;
static bit		muteupevent	= 0;
static bit		locked		= 0;
static bit		start_button	= 0;
static bit		setup_button	= 0;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void userinterface_init (void)
/******************************************************************************/
/* Function:	userinterface_init					      */
/*		- Initialize the CatGenius user interface		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* userinterface_init */


void userinterface_work (void)
/******************************************************************************/
/* Function:	userinterface_work					      */
/*		- Worker function for the CatGenius user interface	      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* userinterface_work */


void userinterface_term (void)
/******************************************************************************/
/* Function:	userinterface_term					      */
/*		- Terminate the CatGenius user interface		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* userinterface_term */


void startbutton_event (unsigned char up)
/******************************************************************************/
/* Function:	startbutton_event					      */
/*		- Handle state changes of Start button			      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (!up) {
		start_button = 1;
		settimeout(&holdtimeout, HOLDTIME);
		if (!locked) {
		} else {
			set_Beeper(0x01, 0);
			if (setup_button)
				muteupevent = 1;
		}
	} else {
		start_button = 0;
		if (!setup_button);
	}
}
/* startbutton_event */


void setupbutton_event (unsigned char up)
/******************************************************************************/
/* Function:	setupbutton_event					      */
/*		- Handle state changes of Setup button			      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (!up) {
		setup_button = 1;
		settimeout(&holdtimeout, HOLDTIME);
	} else {
		setup_button = 0;
	}
}
/* setupbutton_event */


