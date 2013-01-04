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

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "userinterface.h"
#include "../common/timer.h"
#include "../common/rtc.h"
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


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	cartridgetimeout= EXPIRED;
static struct timer	holdtimeout	= NEVER;
static struct timer	autotimer	= NEVER;
static struct timer	cattimer	= EXPIRED;

/* Keyboard status bits */
static unsigned char	buttonmask_cur	= 0;
static unsigned char	buttonmask_cum	= 0;
static unsigned char	buttonmask_evt	= 0;
static bit		locked		= 0;
static bit		longhandled	= 0;

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
static void	process_button		(unsigned char	button_mask,
					 unsigned char	down);
static void	setup_short		(void);
static void	setup_long		(void);
static void	start_short		(void);
static void	start_long		(void);
static void	both_short		(void);
static void	both_long		(void);
static void	update_autotimer	(unsigned char mode);


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
	if ((flags & START_BUTTON) &&
	    (flags & SETUP_BUTTON)) {
		/* User wants to reset non-volatile settings */
		eeprom_write(NVM_MODE, AUTO_MANUAL);
		eeprom_write(NVM_KEYUNDLOCK, 0xFF);
	}

	/* Restore key lock mode from eeprom */
	locked = !eeprom_read(NVM_KEYUNDLOCK);
	/* Restore current mode from eeprom */
	set_mode(eeprom_read(NVM_MODE));
	/* Update the display */
	update_display();
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
	unsigned char		update		= 0;

	if( (disp_mode == DISP_CARTRIDGELEVEL) &&
	    (timeoutexpired(&cartridgetimeout)) ) {
		if (error_nr)
			disp_mode = DISP_ERROR;
		else
			disp_mode = DISP_AUTOMODE;
		update = 1;
	}

	switch(state) {
	default:
		state = STATE_IDLE;
	case STATE_IDLE:
		/* Check if it's time for a timed wash */
		if (timeoutexpired(&autotimer)) {
			/* Schedule the next timed wash */
			update_autotimer(auto_mode);
			printtime();
			DBG("Autotimer expired: ");
			/* Skip the actual washing is no cat has been detected */
			if (cat_detected) {
				DBG("waiting...\n");
				state = STATE_CAT;
				update = 1;
			} else
				DBG("skipping\n");
		}
		break;
	case STATE_CAT:
		/* Wait until the cat has gone */
		if (!cat_present && timeoutexpired(&cattimer)) {
			printtime();
			DBG("Cattimer expired\n");
			litterlanguage_start(full_wash);
			state = STATE_RUNNING;

			/* Update the display (to stop Cat LED from blinking) */
			update = 1;
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
			update = 1;
		}
		break;
	}

	/* Handle pressed buttons */
	switch (buttonmask_evt & BUTTONS) {
	case 0:
		/* No button pressed */
		break;

	case START_BUTTON:
		start_short();
		update = 1;
		break;

	case SETUP_BUTTON:
		setup_short();
		update = 1;
		break;

	case (START_BUTTON | SETUP_BUTTON):
		both_short();
		update = 1;
		break;

	default:
		break;
	}
	buttonmask_evt = 0;

	/* Handle held buttons */
	if (timeoutexpired(&holdtimeout)) {
		switch (buttonmask_cum & BUTTONS) {
		case START_BUTTON:
			set_Beeper(0x05, 0);
			start_long();
			update = 1;
			break;

		case SETUP_BUTTON:
			set_Beeper(0x05, 0);
			setup_long();
			update = 1;
			break;

		case (START_BUTTON | SETUP_BUTTON):
			set_Beeper(0x05, 0);
			both_long();
			update = 1;
			break;

		default:
			break;
		}
		longhandled = 1;
		timeoutnever(&holdtimeout);
	}

	if (update)
		update_display();

}
/* userinterface_work */


void startbutton_event (unsigned char up)
/******************************************************************************/
/* Function:	startbutton_event					      */
/*		- Handle state changes of Start button			      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	process_button (START_BUTTON, !up);
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
	process_button (SETUP_BUTTON, !up);
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

	printtime();
	DBG("Cat %s\n", detected?"in":"out");

	/* Trigger detection on rising edge only */
	if (detected)
		cat_detected = 1;

	/* Update cat timeout once detection is eminent */
	if (cat_detected)
		if (detected)
			timeoutnever(&cattimer);
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


void litterlanguage_event (unsigned char event, unsigned char active)
{
	switch (event) {
	case EVENT_LEVEL_CHANGED:
		break;
	case EVENT_ERR_FILLING:
		if (active)
			set_Beeper(0x01, 1);
		break;
	case EVENT_ERR_DRAINING:
		if (active)
			set_Beeper(0x05, 1);
		break;
	case EVENT_ERR_OVERHEAT:
		if (active)
			set_Beeper(0x15, 1);
		break;
	case EVENT_ERR_EXECUTION:
		break;
	case EVENT_ERR_FLOOD:
		break;
	default:
		break;
	}
}
/* litterlanguage_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void process_button (unsigned char button_mask, unsigned char down)
{
	/* Check for duplicate events */
	if (((buttonmask_cur & button_mask)?1:0) == down)
		return;

	if (down) {
		if (!locked)
			set_Beeper(0x01, 0);
		/* (Re-)Set long-press timeout */
		settimeout(&holdtimeout, HOLDTIME);
		longhandled = 0;
		/* Add the button to the both curent and cumulative masks */
		buttonmask_cur |= button_mask;
		buttonmask_cum |= button_mask;
	} else {
		/* Disable long-press timeout */
		timeoutnever(&holdtimeout);
		/* Remove the button from the current masks */
		buttonmask_cur &= ~button_mask;
		if (!buttonmask_cur) {
			if( (!locked) && (!longhandled) )
				/* Set the cumulative mask as the button event */
				buttonmask_evt = buttonmask_cum;
			/* Reset the cumulative mask */
			buttonmask_cum = 0;
		}
	}
	if (locked)
		update_display();
}

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

	/* Keyboard lock indicator */
	if (locked) {
		if (buttonmask_cur)
			set_LED_Locked(0xAA, 1);
		else
			set_LED_Locked(0xFF, 1);
	} else
		set_LED_Locked(0x00, 0);
}

static void setup_short (void)
{
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
		break;

	case DISP_ERROR:
		break;
	}
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
	} else
		/* Toggle pause current program */
		litterlanguage_pause(!litterlanguage_paused());
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
}


static void both_long (void)
{
	locked = !locked;
	eeprom_write(NVM_KEYUNDLOCK, !locked);
}


static void update_autotimer (unsigned char mode)
{
	/*
	 * TODO: Functions settimeout and settimeout take up to 9.5 hours worth
	 * of seconds, depending on the CPU frequency. So set, and postpone if
	 * nessecary. This needs to be fixed to accomodate higher CPU
	 * frequencies.
	 */
	switch (mode) {
	case AUTO_TIMED1:
		settimeout(&autotimer, 9 * 60 * 60 * SECOND);
		postponetimeout(&autotimer, 9 * 60 * 60 * SECOND);
		postponetimeout(&autotimer, 6 * 60 * 60 * SECOND);
		break;
	case AUTO_TIMED2:
		settimeout(&autotimer, 9 * 60 * 60 * SECOND);
		postponetimeout(&autotimer, 3 * 60 * 60 * SECOND);
		break;
	case AUTO_TIMED3:
		settimeout(&autotimer,  8 * 60 * 60 * SECOND);
		break;
	case AUTO_TIMED4:
		settimeout(&autotimer,  6 * 60 * 60 * SECOND);
		break;
	default:
		timeoutnever(&autotimer);
		break;
	}
}
