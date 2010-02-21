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

static struct timer	timer     = {0xFFFF, 0xFFFFFFFF};
static unsigned char	scooponly = 1;
static unsigned char	state     = 0;


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
		timeoutnever(&timer);
		state = 0;
	/*
	 * First scoop
	 */
	case 0:	/* Wait for program to start */
		if (timeoutexpired(&timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&timer, 13153*MILISECOND);
			state++;
		}
		break;
	case 1: /* Wait for arm to hit bottom */
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 18109*MILISECOND);
			state++;
		}
		break;
	case 2:
		if (timeoutexpired(&timer)) {
			set_Bowl(BOWL_CW);
			settimeout(&timer, 6201*MILISECOND);
			state++;
		}
		break;
	case 3:
		if (timeoutexpired(&timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&timer, 5669*MILISECOND);
			state++;
		}
		break;
	case 4:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 531*MILISECOND);
			state++;
		}
		break;
	case 5:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 25206*MILISECOND);
			state++;
		}
		break;
	case 6:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 10670*MILISECOND);
			state++;
		}
		break;
	case 7:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&timer, 6601*MILISECOND);
			state++;
		}
	case 8:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 17204*MILISECOND);
			state++;
		}
		break;
	/*
	 * Second scoop
	 */
	case 9:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&timer, 12703*MILISECOND);
			state++;
		}
		break;
	case 10:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 4669*MILISECOND);
			state++;
		}
		break;
	case 11:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&timer, 11170*MILISECOND);
			state++;
		}
		break;
	case 12:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 531*MILISECOND);
			state++;
		}
		break;
	case 13:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 25206*MILISECOND);
			state++;
		}
		break;
	case 14:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 10670*MILISECOND);
			state++;
		}
		break;
	case 15:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&timer, 6601*MILISECOND);
			state++;
		}
		break;
	case 16:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 20141*MILISECOND);
			state++;
		}
		break;
	/*
	 * Third scoop
	 */
	case 17:
		if (timeoutexpired(&timer)) {
			set_Bowl(BOWL_CW);
			set_Arm(ARM_DOWN);
			settimeout(&timer, 21705*MILISECOND);
			state++;
		}
		break;
	case 18:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 932*MILISECOND);
			state++;
		}
		break;
	case 19:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 12107*MILISECOND);
			state++;
		}
		break;
	case 20:
		if (timeoutexpired(&timer)) {
			set_Bowl(BOWL_CCW);
			set_Arm(ARM_DOWN);
			settimeout(&timer, 3168*MILISECOND);
			state++;
		}
	case 21:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 531*MILISECOND);
			state++;
		}
		break;
	case 22:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			settimeout(&timer, 24206*MILISECOND);
			state++;
		}
		break;
	case 23:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			settimeout(&timer, 10571*MILISECOND);
			state++;
		}
		break;
	case 24:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_DOWN);
			settimeout(&timer, 6602*MILISECOND);
			state++;
		}
		break;
	case 25:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_UP);
			if (!scooponly)
				set_Water(1);
			set_Bowl(BOWL_CCW);
			settimeout(&timer, 17141*MILISECOND);
			state++;
		}
		break;
	case 26:
		if (timeoutexpired(&timer)) {
			set_Arm(ARM_STOP);
			if (scooponly) {
				set_Bowl(BOWL_STOP);
				timeoutnever(&timer);
				state = 0;
			} else {
				settimeout(&timer, 18673*MILISECOND);
				state++;
			}
		}
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
		timeoutnow(&timer);
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

