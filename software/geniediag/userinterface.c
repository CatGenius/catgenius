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

#include "../common/timer.h"
#include "../common/water.h"
#include "userinterface.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define HOLDTIME		(2 * SECOND)	/* Consider 2 second a long button press*/

#define	ACT_BOWL		0
#define ACT_ARM			1
#define ACT_DOSAGE		2
#define ACT_PUMP		3
#define ACT_DRYER		4
#define ACT_WATER		5

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	holdtimeout	= NEVER;

/* Event statuses */
bit			cat_detected	= 0;
bit			overheated	= 0;

/* Keyboard status bits */
static unsigned char	buttonmask_cur	= 0;
static unsigned char	buttonmask_cum	= 0;
static unsigned char	buttonmask_evt	= 0;
static bit		locked		= 0;
static bit		longhandled	= 0;

static unsigned char	actuator	= 0;
static unsigned char	bowl		= 0;
static unsigned char	arm		= 0;
static unsigned char	dosage		= 0;
static unsigned char	pump		= 0;
static unsigned char	dryer		= 0;
static unsigned char	water		= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void	update_display		(void);
static void	process_button		(unsigned char	button_mask,
					 unsigned char	down);


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
	set_LED(1, 1);
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
	cat_detected = detected;

	DBG("Cat: %s\n", cat_detected?"in":"out");

	if (cat_detected)
		set_LED_Cat(0xFF, 1);
	else
		set_LED_Cat(0x00, 1);
}
/* catsensor_event */


void watersensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	DBG("Water: %s\n", detected?"high":"low");

	if (detected)
		set_LED_Error(0xFF, 1);
	else
		set_LED_Error(0x00, 1);
}
/* watersensor_event */


void heatsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	heatsensor_event					      */
/*		- Handle state changes of over-heat sensor		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	overheated = detected;

	DBG("Overheat: %s\n", overheated?"yes":"no");
}
/* heatsensor_event */


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

static void update_display (void)
{
	/* Mode indication */
	switch (actuator) {
	case ACT_BOWL:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 0);
		set_LED(4, 0);
		break;
	case ACT_ARM:
		set_LED(1, 0);
		set_LED(2, 1);
		set_LED(3, 0);
		set_LED(4, 0);
		break;
	case ACT_DOSAGE:
		set_LED(1, 1);
		set_LED(2, 1);
		set_LED(3, 0);
		set_LED(4, 0);
		break;
	case ACT_PUMP:
		set_LED(1, 0);
		set_LED(2, 0);
		set_LED(3, 1);
		set_LED(4, 0);
		break;
	case ACT_DRYER:
		set_LED(1, 1);
		set_LED(2, 0);
		set_LED(3, 1);
		set_LED(4, 0);
		break;
	case ACT_WATER:
		set_LED(1, 0);
		set_LED(2, 1);
		set_LED(3, 1);
		set_LED(4, 0);
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

void setup_short (void)
{
	if (++actuator > ACT_WATER)
		actuator = ACT_BOWL;

	water_ledalwayson(actuator == ACT_WATER);

	printf ("Mode: ");
	switch (actuator) {
	case ACT_BOWL:
		printf ("bowl");
		break;
	case ACT_ARM:
		printf ("arm");
		break;
	case ACT_DOSAGE:
		printf ("dosage");
		break;
	case ACT_PUMP:
		printf ("drain");
		break;
	case ACT_DRYER:
		printf ("dryer");
		break;
	case ACT_WATER:
		printf ("tap");
		break;
	}
	printf ("\n");
}

void setup_long (void)
{
}

void start_short (void)
{
	switch (actuator) {
		case ACT_BOWL:
			if (++bowl > BOWL_CCW)
				bowl = BOWL_STOP;
			set_Bowl(bowl);
			printf("Bowl: ");
			switch (get_Bowl()) {
			case BOWL_STOP:
				printf("stop");
				break;
			case BOWL_CW:
				printf("cw");
				break;
			case BOWL_CCW:
				printf("ccw");
				break;
			default:
				printf("<unknown>");
				break;
			}
			printf("\n");
			break;
		case ACT_ARM:
			if (++arm > ARM_UP)
				arm = ARM_STOP;
			set_Arm(arm);
			printf("Arm: ");
			switch (get_Arm()) {
			case ARM_STOP:
				printf("stop");
				break;
			case ARM_UP:
				printf("up");
				break;
			case ARM_DOWN:
				printf("down");
				break;
			default:
				printf("<unknown>");
				break;
			}
			printf("\n");
			break;
		case ACT_DOSAGE:
			if (++dosage > 1)
				dosage = 0;
			set_Dosage(dosage);
			printf("Dosage: %s\n", get_Dosage()?"on":"off");
			break;
		case ACT_PUMP:
			if (++pump > 1)
				pump = 0;
			set_Pump(pump);
			printf("Drain: %s\n", get_Pump()?"on":"off");
			break;
		case ACT_DRYER:
			if (++dryer > 1)
				dryer = 0;
			set_Dryer(dryer);
			printf("Dryer: %s\n", get_Dryer()?"on":"off");
			break;
		case ACT_WATER:
			if (++water > 1)
				water = 0;
			water_fill(water);
			printf("Tap: %s\n", water_filling()?"on":"off");
			break;
	}
}

void start_long (void)
{
	printf("Bowl: stop\n");
	bowl = BOWL_STOP;
	set_Bowl(bowl);
	printf("Arm: stop\n");
	arm = ARM_STOP;
	set_Arm(arm);
	dosage = 0;
	set_Dosage(dosage);
	printf("Dosage: off\n");
	pump = 0;
	set_Pump(pump);
	printf("Drain: off\n");
	dryer = 0;
	set_Dryer(dryer);
	printf("Dryer: off\n");
	water = 0;
	water_fill(water);
	printf("Tap: off\n");
}

void both_short (void)
{
}

void both_long (void)
{
	locked = !locked;
	printf("Lock: %s\n", locked?"on":"off");
}
