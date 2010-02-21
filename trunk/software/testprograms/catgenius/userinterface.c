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

static unsigned char	mode		= 0;
static unsigned char		program_scooponly = 1;
static unsigned char	program_state	= 0;
static struct timer	program_timer	= {0xFFFF, 0xFFFFFFFF};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		setup	(void);
static void		start	(void);
static void		level	(void);
static void		lock	(void);

static void program_work (void);


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
	program_work();
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
	if (++mode > 9)
		mode = 0;
	switch (mode) {
	default:
		mode = 0;
	case 0:
		set_LED(1, 0);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0x00, 0);
		break;
	case 1:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0x00, 0);
		break;
	case 2:
		set_LED(1, 0);
		set_LED(2, 1);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0x00, 0);
		break;
	case 3:
		set_LED(1, 0);
		set_LED(2, 0);
		set_LED(3, 1);
		set_LED(4, 0);
		set_LED_Cat(0x00, 0);
		break;
	case 4:
		set_LED(1, 0);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 1);
		set_LED_Cat(0x00, 0);
		break;
	case 5:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0xFF, 1);
		break;
	case 6:
		set_LED(1, 1);
		set_LED(2, 1);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0xFF, 1);
		break;
	case 7:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 1);
		set_LED(4, 0);
		set_LED_Cat(0xFF, 1);
		break;
	case 8:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 1);
		set_LED_Cat(0xFF, 1);
		break;
	case 9:
		set_LED(1, 0);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 0);
		set_LED_Cat(0xFF, 1);
		break;
	}
}


static void start (void)
{
	if (!program_state)
		timeoutnow(&program_timer);
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

static void program_work (void)
{
	switch (program_state) {
	default:
		set_Arm(ARM_STOP);
		set_Bowl(BOWL_STOP);
		timeoutnever(&program_timer);
		program_state = 0;
	/*
	 * First scoop
	 */
	case 0:	/* Wait for program to start */
		if (timeoutexpired(&program_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 13153*MILISECOND);
			program_state++;
		}
		break;
	case 1: /* Wait for arm to hit bottom */
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 18109*MILISECOND);
			program_state++;
		}
		break;
	case 2:
		if (timeoutexpired(&program_timer)) {
			set_Bowl(BOWL_CW);
			settimeout(&program_timer, 6201*MILISECOND);
			program_state++;
		}
		break;
	case 3:
		if (timeoutexpired(&program_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 5669*MILISECOND);
			program_state++;
		}
		break;
	case 4:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 531*MILISECOND);
			program_state++;
		}
		break;
	case 5:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 25206*MILISECOND);
			program_state++;
		}
		break;
	case 6:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 10670*MILISECOND);
			program_state++;
		}
		break;
	case 7:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 6601*MILISECOND);
			program_state++;
		}
	case 8:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 17204*MILISECOND);
			program_state++;
		}
		break;
	/*
	 * Second scoop
	 */
	case 9:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 12703*MILISECOND);
			program_state++;
		}
		break;
	case 10:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 4669*MILISECOND);
			program_state++;
		}
		break;
	case 11:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 11170*MILISECOND);
			program_state++;
		}
		break;
	case 12:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 531*MILISECOND);
			program_state++;
		}
		break;
	case 13:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 25206*MILISECOND);
			program_state++;
		}
		break;
	case 14:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 10670*MILISECOND);
			program_state++;
		}
		break;
	case 15:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 6601*MILISECOND);
			program_state++;
		}
		break;
	case 16:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 20141*MILISECOND);
			program_state++;
		}
		break;
	/*
	 * Third scoop
	 */
	case 17:
		if (timeoutexpired(&program_timer)) {
			set_Bowl(BOWL_CW);
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 21705*MILISECOND);
			program_state++;
		}
		break;
	case 18:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 932*MILISECOND);
			program_state++;
		}
		break;
	case 19:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 12107*MILISECOND);
			program_state++;
		}
		break;
	case 20:
		if (timeoutexpired(&program_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 3168*MILISECOND);
			program_state++;
		}
	case 21:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 531*MILISECOND);
			program_state++;
		}
		break;
	case 22:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&program_timer, 24206*MILISECOND);
			program_state++;
		}
		break;
	case 23:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			settimeout(&program_timer, 10571*MILISECOND);
			program_state++;
		}
		break;
	case 24:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&program_timer, 6602*MILISECOND);
			program_state++;
		}
		break;
	case 25:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_UP);
			if (!program_scooponly)
				set_Water(1);
			set_Bowl(BOWL_CCW);
			settimeout(&program_timer, 17141*MILISECOND);
			program_state++;
		}
		break;
	case 26:
		if (timeoutexpired(&program_timer)) {
			set_Arm(ARM_STOP);
			if (program_scooponly) {
				set_Bowl(BOWL_STOP);
				timeoutnever(&program_timer);
				program_state = 0;
			} else {
				settimeout(&program_timer, 18673*MILISECOND);
				program_state++;
			}
		}
		break;
	}
}