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
#include "litterlanguage.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define HOLDTIME		(3 * SECOND)
#define LEVEL_TIMEOUT		(2 * SECOND)

#define DISP_AUTOMODE		0
#define DISP_CARTRIDGELEVEL	1
#define DISP_ERROR		2

#define AUTO_MANUAL		0
#define AUTO_TIMED1		1
#define AUTO_TIMED2		2
#define AUTO_TIMED3		3
#define AUTO_TIMED4		4
#define AUTO_DETECTED1ON1	5
#define AUTO_DETECTED1ON2	6
#define AUTO_DETECTED1ON3	7
#define AUTO_DETECTED1ON4	8
#define AUTO_DETECTED		9

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	cartridgetimeout= EXIRED;
static struct timer	holdtimeout	= NEVER;

/* Keyboard status bits */
static bit		muteupevent	= 0;
static bit		locked		= 0;
static bit		start_button	= 0;
static bit		setup_button	= 0;

static unsigned char	disp_mode	= DISP_AUTOMODE;
static unsigned char	auto_mode	= AUTO_MANUAL;
static unsigned char	cart_level	= 100;
static unsigned char	error_nr	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		update_display	(unsigned char mode);
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
	if( (disp_mode == DISP_CARTRIDGELEVEL) &&
	    (timeoutexpired(&cartridgetimeout)) ) {
		if (error_nr)
			disp_mode = DISP_ERROR;
		else
			disp_mode = DISP_AUTOMODE;
		update_display(disp_mode);
	}
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

static void update_display (unsigned char mode)
{
	if (mode != disp_mode)
		return;
	switch (disp_mode) {
	default:
		disp_mode = DISP_AUTOMODE;
	case DISP_AUTOMODE:
		set_LED_Error(0x00, 0);
		if (cart_level >= 10)
			set_LED_Cartridge(0x00, 0);
		else
			set_LED_Cartridge(0x55, 1);
		switch (auto_mode) {
		default:
			auto_mode = 0;
		case AUTO_MANUAL:
			set_LED(1, 0);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_TIMED1:
			set_LED(1, 1);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_TIMED2:
			set_LED(1, 0);
			set_LED(2, 1);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_TIMED3:
			set_LED(1, 0);
			set_LED(2, 0);
			set_LED(3, 1);
			set_LED(4, 0);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_TIMED4:
			set_LED(1, 0);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 1);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_DETECTED1ON1:
			set_LED(1, 1);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0xFF, 1);
			break;
		case AUTO_DETECTED1ON2:
			set_LED(1, 1);
			set_LED(2, 1);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0xFF, 1);
			break;
		case AUTO_DETECTED1ON3:
			set_LED(1, 1);
			set_LED(2, 0);
			set_LED(3, 1);
			set_LED(4, 0);
			set_LED_Cat(0xFF, 1);
			break;
		case AUTO_DETECTED1ON4:
			set_LED(1, 1);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 1);
			set_LED_Cat(0xFF, 1);
			break;
		case AUTO_DETECTED:
			set_LED(1, 0);
			set_LED(2, 0);
			set_LED(3, 0);
			set_LED(4, 0);
			set_LED_Cat(0xFF, 1);
			break;
		}
		break;
	case DISP_CARTRIDGELEVEL:
		set_LED_Error(0x00, 0);
		set_LED_Cartridge(0xFF, 1);
		set_LED(1, cart_level >= 10);
		set_LED(2, cart_level >= 25);
		set_LED(3, cart_level >= 50);
		set_LED(4, cart_level >= 75);
		if (cart_level >= 100)
			set_LED_Cat(0xFF, 1);
		else
			set_LED_Cat(0x00, 0);
		break;
	case DISP_ERROR:
		set_LED_Error(0x55, 1);
		set_LED_Cartridge(0x00, 0);
		set_LED(1, error_nr == 1);
		set_LED(2, error_nr == 2);
		set_LED(3, error_nr == 3);
		set_LED(4, error_nr == 4);
		break;
	}
}


static void setup (void)
{
	switch (disp_mode) {
	default:
		disp_mode = DISP_AUTOMODE;
	case DISP_AUTOMODE:
		/* Increase auto mode */
		if (++auto_mode > AUTO_DETECTED)
			auto_mode = AUTO_MANUAL;
		/* Reset timers and cat sensor */

		/* Update the display */
		update_display(disp_mode);
		break;

	case DISP_CARTRIDGELEVEL:
		if (cart_level < 10)
			cart_level = 10;
		else if (cart_level < 25)
			cart_level = 25;
		else if (cart_level < 50)
			cart_level = 50;
		else if (cart_level < 75)
			cart_level = 75;
		else if (cart_level < 100)
			cart_level = 100;
		else
			cart_level = 0;
		/* Set new timeout to return to auto- or errormode */
		settimeout(&cartridgetimeout, LEVEL_TIMEOUT);
		/* Update the display */
		update_display(disp_mode);
		break;

	case DISP_ERROR:
		break;
	}
}

static void start (void)
{
	if (!litterlanguage_running())
		litterlanguage_start();
}


static void level (void)
{
	/* Swich display to cartridge level mode */
	disp_mode = DISP_CARTRIDGELEVEL;
	/* Set timeout to return to auto- or errormode */
	settimeout(&cartridgetimeout, LEVEL_TIMEOUT);
	/* Update the display */
	update_display(disp_mode);
}


static void lock (void)
{
	locked = !locked;
	if (locked)
		set_LED_Locked(0xFF, 1);
	else
		set_LED_Locked(0x00, 0);
}
