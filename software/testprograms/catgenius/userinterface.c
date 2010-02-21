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

static struct timer	holdtimeout	= {0xFFFF, 0xFFFFFFFF};
static bit		muteupevent	= 0;
static bit		locked		= 0;
static bit		start_button	= 0;
static bit		setup_button	= 0;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		setup	(void);
static void		start	(void);
static void		level	(void);
static void		lock	(void);


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
		if (setup_button)
			muteupevent = 1;
		if (!locked)
			set_Beeper(0x01, 0);
		else
			set_LED_Locked(0x55, 1);
	} else {
		start_button = 0;
		if (!muteupevent) 
			if (locked)
				set_LED_Locked(0xFF, 1);
			else
				/* Handle setup up */
				start();
		else
			if (!setup_button) {
				if (timeoutexpired(&holdtimeout))
					/* Handle long double-press up */
					lock();
				else
					/* Handle short double-press up */
					if (locked)
						set_LED_Locked(0xFF, 1);
					else
						/* Handle level check */
						level();
				muteupevent = 0;
			}
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
		if (start_button)
			muteupevent = 1;
		if (!locked)
			set_Beeper(0x01, 0);
		else
			set_LED_Locked(0x55, 1);
	} else {
		setup_button = 0;
		if (!muteupevent)
			if (locked)
				set_LED_Locked(0xFF, 1);
			else
				/* Handle setup up */
				setup();
		else
			if (!start_button) {
				if (timeoutexpired(&holdtimeout))
					/* Handle long double-press up */
					lock();
				else
					/* Handle short double-press up */
					if (locked)
						set_LED_Locked(0xFF, 1);
					else
						/* Handle level check */
						level();
				muteupevent = 0;
			}
	}
}
/* setupbutton_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void setup (void)
{
	set_Beeper(0x05, 0);
}


static void start (void)
{
	set_Beeper(0x05, 0);
}


static void level (void)
{
	set_Beeper(0x15, 0);
}


static void lock (void)
{
	locked = !locked;
	if (locked)
		set_LED_Locked(0xFF, 1);
	else
		set_LED_Locked(0x00, 0);
}
