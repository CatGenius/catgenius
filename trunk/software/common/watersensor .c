/******************************************************************************/
/* File    :	watersensor.c						      */
/* Function:	Water sensor functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	2 Oct 2010 by R. Delien:				      */
/*		- Extracted from catgenie120.c.				      */
/******************************************************************************/
#include <htc.h>

#include "watersensor.h"
#include "hardware.h"
#include "timer.h"

extern void watersensor_event (unsigned char detected);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define WATERSENSORPOLLING	(SECOND/4)	/*  250ms*/
#define MAX_CONSECUTIVE		8


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	water_sensortimer = EXPIRED;
static unsigned char	water_consecutive = 0;
static bit		water_detected    = 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void watersensor_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: watersensor_init */


void watersensor_work (void)
/******************************************************************************/
/* Function:	watersensor_work					      */
/*		- Worker function for the CatGenie 120 hardware		      */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned char	filling;
	unsigned char	loops;
	unsigned char	reflection;

	if (!timeoutexpired(&water_sensortimer))
		return;

	/* Set new polling timeout */
	settimeout(&water_sensortimer, WATERSENSORPOLLING);

	filling = WATERSENSOR_LED_PORT & WATERSENSOR_LED_MASK;
	if (!filling)
		/* Unmute Water Sensor by switching on the LED */
		WATERSENSOR_LED_PORT |= WATERSENSOR_LED_MASK;

	loops = 0;
	do {
		/* 0 == no reflection == water detected */
		if (reflection = WATERSENSOR_PORT & WATERSENSOR_MASK)
			break;
		loops++;
	} while (loops <= 80); /* 1 loop is ~25us */

	if (!filling)
		/* Mute Water Sensor by switching off the LED */
		WATERSENSOR_LED_PORT &= ~WATERSENSOR_LED_MASK;

	if (reflection) {
		if (water_consecutive > 0) {
			water_consecutive--;
			if( (water_consecutive == 0) && water_detected ) {
				water_detected = 0;
				watersensor_event(0);
			}
		}
	} else {
		if (water_consecutive < MAX_CONSECUTIVE) {
			water_consecutive++;
			if( (water_consecutive == MAX_CONSECUTIVE) && !water_detected ) {
				water_detected = 1;
				watersensor_event(1);
			}
		}
	}
}


void watersensor_term (void)
/******************************************************************************/
/* Function:	Module termination routine				      */
/*		- Terminates the module					      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* End: watersensor_term */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
