/******************************************************************************/
/* File    :	washprogram.c						      */
/* Function:	CatGenius washing program				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "washprogram.h"
#include "../common/timer.h"
#include "../common/catgenie120.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	state_timer = {0xFFFF, 0xFFFFFFFF};
static struct timer	water_timer = {0xFFFF, 0xFFFFFFFF};
static unsigned char	scooponly   = 1;
static unsigned char	state       = 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void washprogram_init (void)
/******************************************************************************/
/* Function:	washprogram_init					      */
/*		- Initialize the CatGenius washing program		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* washprogram_init */

#define MAX_FILL_TIME	(135*SECOND)
#define SCOOP		0
#define WASH		27
#define DRY		100
#define EQUALIZE	120

void washprogram_work (void)
/******************************************************************************/
/* Function:	washprogram_work					      */
/*		- Worker function for the CatGenius washing program	      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	switch (state) {
	default:
		set_Arm(ARM_STOP);
		set_Bowl(BOWL_STOP);
		timeoutnever(&state_timer);
		state = 0;
	/*
	 * Scooping
	 */
	case SCOOP:	/* Wait for program to start */
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 13153*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 1: /* Wait for arm to hit bottom */
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 18109*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 2:
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CW);
			settimeout(&state_timer, 6201*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 3:
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 5669*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 4:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 531*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 5:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 25206*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 6:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 10670*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 7:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 6601*MILISECOND);
			state++;
		}
	case SCOOP + 8:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 17204*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 9:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 12703*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 10:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 4669*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 11:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 11170*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 12:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 531*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 13:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 25206*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 14:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 10670*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 15:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 6601*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 16:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 20141*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 17:
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CW);
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 21705*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 18:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 932*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 19:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 12107*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 20:
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 3168*MILISECOND);
			state++;
		}
	case SCOOP + 21:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 531*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 22:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 24206*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 23:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 10571*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 24:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 6602*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 25:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			if (!scooponly) {
				set_Water(1);
				settimeout(&water_timer, MAX_FILL_TIME);
			}
			set_Bowl(BOWL_CW);
			settimeout(&state_timer, 17141*MILISECOND);
			state++;
		}
		break;
	case SCOOP + 26:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			if (scooponly) {
				set_Bowl(BOWL_STOP);
				settimeout(&state_timer, 0*MILISECOND); /* TODO */
				state = EQUALIZE;
			} else {
				set_Bowl(BOWL_CW);
				settimeout(&state_timer, 18673*MILISECOND);
				state = WASH;
			}
		}
		break;
	/*
	 * Washing
	 */
	case WASH:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&state_timer, 25174*MILISECOND);
			state++;
		}
		break;
	case WASH + 1:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_UP);
			settimeout(&state_timer, 1132*MILISECOND);
			state++;
		}
		break;
	case WASH + 2:
		if (timeoutexpired(&state_timer)) {
			set_Arm(ARM_STOP);
			settimeout(&state_timer, 1132*MILISECOND);
			state++;
		}
		break;
	case WASH + 3:
		if (detected_Water()) {
			set_Water(0);
			settimeout(&state_timer, 63582*MILISECOND);
			state++;
		} else
			if (timeoutexpired(&water_timer)) {
				set_Water(0);
				/*
				 * ERROR 2
				 */
			}
		break;
	case WASH + 4:
		if (timeoutexpired(&state_timer)) {
			set_Pump(1);
			settimeout(&state_timer, 25602*MILISECOND);
			state++;
		}
		break;
	case WASH + 5:
		if (timeoutexpired(&state_timer)) {
			set_Pump(0);
			settimeout(&state_timer, 75718*MILISECOND);
			state++;
		}
		break;
	case WASH + 6:
		if (timeoutexpired(&state_timer)) {
			set_Pump(1);
			settimeout(&state_timer, 25602*MILISECOND);
			state++;
		}
		break;
	case WASH + 7:
		if (timeoutexpired(&state_timer)) {
			set_Pump(0);
			settimeout(&state_timer, 8202*MILISECOND);
			state++;
		}
		break;
	case WASH + 8:
		if (timeoutexpired(&state_timer)) {
			set_Pump(1);
			settimeout(&state_timer, 25602*MILISECOND);
			state++;
		}
		break;
	case WASH + 9:
		if (timeoutexpired(&state_timer)) {
			set_Pump(0);
			settimeout(&state_timer, 8202*MILISECOND);
			state++;
		}
		break;
	case WASH + 10:
		if (timeoutexpired(&state_timer)) {
			set_Pump(1);
			settimeout(&state_timer, 65616*MILISECOND);
			state++;
		}
		break;
	case WASH + 11:
		if (timeoutexpired(&state_timer)) {
			set_Pump(0);
			set_Water(1);
			settimeout(&water_timer, MAX_FILL_TIME);
			settimeout(&state_timer, 55418*MILISECOND);
			state++;
		}
		break;
	case WASH + 12:
		if (timeoutexpired(&state_timer)) {
			set_Bowl(BOWL_CCW);
			set_Dosage(1);
			settimeout(&water_timer, MAX_FILL_TIME);
			settimeout(&state_timer, 2462*MILISECOND);
			state++;
		}
		break;
	case WASH + 13:
		if (timeoutexpired(&state_timer)) {
			set_Dosage(0);
			set_Bowl(BOWL_CW);
			settimeout(&water_timer, MAX_FILL_TIME);
			settimeout(&state_timer, 0*MILISECOND);
			state++;
		}
		break;
	case WASH + 14:
		if (detected_Water()) {
			set_Water(0);
			settimeout(&state_timer, 63582*MILISECOND);
			state++;
		} else
			if (timeoutexpired(&water_timer)) {
				set_Water(0);
				/*
				 * ERROR 2
				 */
			}
		break;
	/*
	 * Drying
	 */
	case DRY:
		break;
	/*
	 * Equalizing
	 */
	case EQUALIZE:
		break;
	}
}
/* washprogram_work */


void washprogram_term (void)
/******************************************************************************/
/* Function:	userinterface_term					      */
/*		- Terminate the CatGenius washing program		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* washprogram_term */


void washprogram_mode (unsigned char justscoop)
{
	if (!state)
		scooponly = justscoop ;
}


void washprogram_start (void)
{
	if (!state)
		timeoutnow(&state_timer);
}


unsigned char washprogram_running (void)
{
	return (state);
}


void washprogram_pause (unsigned char pause)
{
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

