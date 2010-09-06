/******************************************************************************/
/* File    :	userinterface.c						      */
/* Function:	CatGenie user interface					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "userinterface.h"
#include "../common/timer.h"
#include "../common/hardware.h"
#include "../common/catgenie120.h"
#include "litterlanguage.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define HOLDTIME		(2 * SECOND)	/* Consider 2 second a long button press*/
#define LEVEL_TIMEOUT		(5 * SECOND)	/* Show the level for 5 seconds */
#define CAT_TIMEOUT		(4 * 60 * SECOND)

#define DISP_AUTOMODE		0
#define DISP_CARTRIDGELEVEL	1
#define DISP_ERROR		2

#define STATE_IDLE		0
#define STATE_CAT		1
#define STATE_RUNNING		2

#define AUTO_MANUAL		0	/* Nothing automatic, manual only */
#define AUTO_TIMED1		1	/* Full wash every 24 hours */
#define AUTO_TIMED2		2	/* Full wash every 12 hours */
#define AUTO_TIMED3		3	/* Full wash every 8 hours */
#define AUTO_TIMED4		4	/* Full wash every 6 hours */
#define AUTO_DETECTED1ON1	5	/* Full wash every use */
#define AUTO_DETECTED1ON2	6	/* Full wash/Scoop only 1:2 uses */
#define AUTO_DETECTED1ON3	7	/* Full wash/Scoop only 1:3 uses */
#define AUTO_DETECTED1ON4	8	/* Full wash/Scoop only 1:4 uses */
#define AUTO_DETECTED		9	/* Scoop only every use*/


#define update_autotimer(mode)						\
do {									\
	switch (mode) {						\
	case AUTO_TIMED1:						\
		settimeout(&autotimer, 24 * 60 * 60 * SECOND);		\
		break;							\
	case AUTO_TIMED2:						\
		settimeout(&autotimer, 12 * 60 * 60 * SECOND);		\
		break;							\
	case AUTO_TIMED3:						\
		settimeout(&autotimer,  8 * 60 * 60 * SECOND);		\
		break;							\
	case AUTO_TIMED4:						\
		settimeout(&autotimer,  6 * 60 * 60 * SECOND);		\
		break;							\
	default:							\
		timeoutnever(&autotimer);				\
		break;							\
	}								\
} while(0)


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	cartridgetimeout= EXPIRED;
static struct timer	holdtimeout	= NEVER;
static struct timer	autotimer	= NEVER;
static struct timer	cattimer	= EXPIRED;

/* Keyboard status bits */
static bit		muteupevent	= 0;
static bit		locked		= 0;
static bit		start_button	= 0;
static bit		setup_button	= 0;

static bit		cat_present	= 0;
static bit		cat_detected	= 0;
static bit		full_wash	= 0;

static unsigned char	state		= STATE_IDLE;
static unsigned char	interval	= 0;
static unsigned char	disp_mode	= DISP_AUTOMODE;
static unsigned char	auto_mode	= AUTO_MANUAL;
static unsigned char	cart_level	= 100;
static unsigned char	error_nr	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void	set_mode		(unsigned char mode);
static void	update_display		(void);
static void	setup_long		(void);
static void	start_short		(void);
static void	start_long		(void);
static void	both_short		(void);
static void	both_long		(void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void userinterface_init (unsigned char flags)
/******************************************************************************/
/* Function:	userinterface_init					      */
/*		- Initialize the CatGenius user interface		      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if ((flags & START_BUTTON_HELD) &&
	    (flags & SETUP_BUTTON_HELD)) {
		/* User wants to reset non-volatile settings */
		eeprom_write(NVM_MODE, AUTO_MANUAL);
		eeprom_write(NVM_KEYUNDLOCK, 0xFF);
	}

	/* Restore key lock mode from eeprom */
	locked = !eeprom_read(NVM_KEYUNDLOCK);
	/* Restore current mode from eeprom */
	set_mode(eeprom_read(NVM_MODE));
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
		update_display();
	}

	switch(state) {
	default:
		state = STATE_IDLE;
	case STATE_IDLE:
		/* Check if it's time for a timed wash */
		if (timeoutexpired(&autotimer)) {
			/* Schedule the next timed wash */
			update_autotimer(auto_mode);
			DBG("Autotimer expired: ");
			/* Skip the actual washing is no cat has been detected */
			if (cat_detected) {
				DBG("waiting...\n");
				state = STATE_CAT;
				/* Update the display */
				update_display();
			} else
				DBG("skipping\n");
		}
		break;
	case STATE_CAT:
		/* Wait until the cat has gone */
		if (!cat_present && timeoutexpired(&cattimer)) {
			DBG("Cattimer expired\n");
			litterlanguage_start(full_wash);
			state = STATE_RUNNING;

			/* Update the display (to stop Cat LED from blinking) */
			update_display();
		}
		break;
	case STATE_RUNNING:
		/* Wait for the program to end */
		if (!litterlanguage_running()) {
			timeoutnow(&cattimer);
			cat_detected = 0;
			if (full_wash)
				interval = 0;
			else
				interval ++;
			state = STATE_IDLE;
			update_display();
		}
		break;
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
	if (start_button == !up)
		return;
	start_button = !up;

	if (start_button) {
		settimeout(&holdtimeout, HOLDTIME);
		if (setup_button)
			muteupevent = 1;
		if (!locked)
			set_Beeper(0x01, 0);
		else
			set_LED_Locked(0x55, 1);
	} else {
		if (!muteupevent)
			if (locked)
				set_LED_Locked(0xFF, 1);
			else
				if (timeoutexpired(&holdtimeout))
					/* Handle long press up */
					start_long();
				else
					/* Handle short press up */
					start_short();
		else
			if (!setup_button) {
				if (timeoutexpired(&holdtimeout))
					/* Handle long both press up */
					both_long();
				else
					/* Handle short both press up */
					if (locked)
						set_LED_Locked(0xFF, 1);
					else
						both_short();
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
	if (setup_button == !up)
		return;
	setup_button = !up;

	if (setup_button) {
		settimeout(&holdtimeout, HOLDTIME);
		if (start_button)
			muteupevent = 1;
		if (!locked)
			set_Beeper(0x01, 0);
		else
			set_LED_Locked(0x55, 1);
	} else {
		if (!muteupevent)
			if (locked)
				set_LED_Locked(0xFF, 1);
			else
				if (timeoutexpired(&holdtimeout))
					/* Handle long press up */
					setup_long();
				else
					/* Handle short press up */
					switch (disp_mode) {
					default:
						disp_mode = DISP_AUTOMODE;
					case DISP_AUTOMODE:
						set_mode((auto_mode==AUTO_DETECTED)?AUTO_MANUAL:auto_mode+1);
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
						update_display();
						break;

					case DISP_ERROR:
						break;
					}
		else
			if (!start_button) {
				if (timeoutexpired(&holdtimeout))
					/* Handle long both press up */
					both_long();
				else
					/* Handle short both press up */
					if (locked)
						set_LED_Locked(0xFF, 1);
					else
						both_short();
				muteupevent = 0;
			}
	}
}
/* setupbutton_event */


void catsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	catsensor_event						      */
/*		- Handle state changes of cat sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Update the actual cat status */
	cat_present = detected;

	DBG("Cat %s\n", detected?"in":"out");

	/* Trigger detection on rising edge only */
	if (detected)
		cat_detected = 1;

	/* Update cat timeout once detection is eminent */
	if (cat_detected)
		if (detected)
			timeoutnever(&cattimer);				\
		else
			settimeout(&cattimer, CAT_TIMEOUT);

	if ( (auto_mode == AUTO_DETECTED1ON1) ||
	     (auto_mode == AUTO_DETECTED1ON2) ||
	     (auto_mode == AUTO_DETECTED1ON3) ||
	     (auto_mode == AUTO_DETECTED1ON4) ||
	     (auto_mode == AUTO_DETECTED) ) {
	     	if (cat_detected && state == STATE_IDLE) {
			switch(auto_mode) {
			case AUTO_DETECTED1ON1:
				full_wash = 1;
				break;
			case AUTO_DETECTED1ON2:
				full_wash = (interval >= 1);
				break;
			case AUTO_DETECTED1ON3:
				full_wash = (interval >= 2);
				break;
			case AUTO_DETECTED1ON4:
				full_wash = (interval >= 3);
				break;
			case AUTO_DETECTED:
				full_wash = 0;
				break;
			}
			state = STATE_CAT;
		}
		/* Update the display */
		update_display();
	}
}
/* catsensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void set_mode (unsigned char mode)
{
	if (mode > AUTO_DETECTED)
		auto_mode = AUTO_MANUAL;
	else
		auto_mode = mode;
	DBG("Set mode %u\n", auto_mode);

	if ( (auto_mode == AUTO_TIMED1) ||
	     (auto_mode == AUTO_TIMED2) ||
	     (auto_mode == AUTO_TIMED3) ||
	     (auto_mode == AUTO_TIMED4) ) {
		full_wash = 1;
	}

	if (state != STATE_RUNNING) {
		/* Reset state machine */
		state = STATE_IDLE;
		interval = 0;
	}
	/* Reset timers and cat sensor */
	update_autotimer(auto_mode);
	timeoutnow(&cattimer);
	cat_detected = 0;

	/* Update the display */
	update_display();

	/* Store the new mode in EEPROM */
	eeprom_write(NVM_MODE, auto_mode);
}

static void update_display (void)
{
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
		case AUTO_TIMED1:
		case AUTO_TIMED2:
		case AUTO_TIMED3:
		case AUTO_TIMED4:
			set_LED(1, auto_mode == AUTO_TIMED1);
			set_LED(2, auto_mode == AUTO_TIMED2);
			set_LED(3, auto_mode == AUTO_TIMED3);
			set_LED(4, auto_mode == AUTO_TIMED4);
			set_LED_Cat(0x00, 0);
			break;
		case AUTO_DETECTED1ON1:
		case AUTO_DETECTED1ON2:
		case AUTO_DETECTED1ON3:
		case AUTO_DETECTED1ON4:
		case AUTO_DETECTED:
			set_LED(1, auto_mode != AUTO_DETECTED);
			set_LED(2, auto_mode == AUTO_DETECTED1ON2);
			set_LED(3, auto_mode == AUTO_DETECTED1ON3);
			set_LED(4, auto_mode == AUTO_DETECTED1ON4);
			if ((cat_present) && (state != STATE_RUNNING))
				set_LED_Cat(0x55, 1);
			else if ((cat_detected) && (state != STATE_RUNNING))
				if (full_wash)
					set_LED_Cat(0xFA, 1);
				else
					set_LED_Cat(0xFE, 1);
			else
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
	if (locked)
		set_LED_Locked(0xFF, 1);
	else
		set_LED_Locked(0x00, 0);
}

static void setup_long (void)
{
}

static void start_short (void)
{
	if (!litterlanguage_running()) {
		/* Scoop-only program */
		full_wash = 0;
		/* Start the program */
		litterlanguage_start(full_wash);
		/* Update the state machine */
		state = STATE_RUNNING;
		/* Update the display */
		update_display();
	} else
		/* Pause current program */
		litterlanguage_pause(0);
}

static void start_long (void)
{
	if (!litterlanguage_running()) {
		/* Full washing program */
		full_wash = 1;
		/* Start the program */
		litterlanguage_start(full_wash);
		/* Update the state machine */
		state = STATE_RUNNING;
		/* Update the display */
		update_display();
	} else
		/* Stop the program, state machine will do the rest */
		litterlanguage_stop();
}

static void both_short (void)
{
	/* Swich display to cartridge level mode */
	disp_mode = DISP_CARTRIDGELEVEL;
	/* Set timeout to return to auto- or errormode */
	settimeout(&cartridgetimeout, LEVEL_TIMEOUT);
	/* Update the display */
	update_display();
}


static void both_long (void)
{
	locked = !locked;
	eeprom_write(NVM_KEYUNDLOCK, !locked);
	/* Update the display */
	update_display();
}
