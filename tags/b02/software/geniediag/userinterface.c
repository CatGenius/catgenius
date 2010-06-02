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

/* Keyboard status bits */
static bit		muteupevent	= 0;
static bit		locked		= 0;
static bit		start_button	= 0;
static bit		setup_button	= 0;

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

static void	setup_short		(void);
static void	setup_long		(void);
static void	start_short		(void);
static void	start_long		(void);
static void	both_short		(void);
static void	both_long		(void);


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
				if (timeoutexpired(&holdtimeout))
					/* Handle long press up */
					setup_long();
				else
					/* Handle short press up */
					setup_short();
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
	DBG("Cat %s\n", detected?"in":"out");

	if (detected)
		set_LED_Cat(0xFF, 1);
	else
		set_LED_Cat(0x00, 1);
}
/* catsensor_event */


void watersensor_event (unsigned char undetected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	DBG("Water %s\n", undetected?"low":"high");

	if (!undetected)
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
	DBG("Overheat %s\n", detected?"yes":"no");
}
/* heatsensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void setup_short (void)
{
	if (++actuator > ACT_WATER)
		actuator = ACT_BOWL;

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
}

static void setup_long (void)
{
}

static void start_short (void)
{
	switch (actuator) {
		case ACT_BOWL:
			if (++bowl > BOWL_CCW)
				bowl = BOWL_STOP;
			set_Bowl(bowl);
			break;
		case ACT_ARM:
			if (++arm > ARM_UP)
				arm = ARM_STOP;
			set_Arm(arm);
			break;
		case ACT_DOSAGE:
			if (++dosage > 1)
				dosage = 0;
			set_Dosage(dosage);
			break;
		case ACT_PUMP:
			if (++pump > 1)
				pump = 0;
			set_Pump(pump);
			break;
		case ACT_DRYER:
			if (++dryer > 1)
				dryer = 0;
			set_Dryer(dryer);
			break;
		case ACT_WATER:
			if (++water > 1)
				water = 0;
			set_Water(water);
			break;
	}
}

static void start_long (void)
{
}

static void both_short (void)
{
}

static void both_long (void)
{
	locked = !locked;
	if (locked)
		set_LED_Locked(0xFF, 1);
	else
		set_LED_Locked(0x00, 0);
}
