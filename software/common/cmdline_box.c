/******************************************************************************/
/* File    :	cmdline_box.c						      */
/* Function:	Command line commands operating the box			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	31 Mar 2012 by R. Delien:				      */
/*		- First creation					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "cmdline.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

/* Event statuses */
extern bit	cat_detected;
extern bit	water_low;
extern bit	overheated;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

int bowl(char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "stop", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_STOP);
		} else if (!strncmp (args, "cw", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_CW);
		} else if (!strncmp (args, "ccw", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_CCW);
		} else
			/* Syntax error */
			return (-1);
	}

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

	return (result);
}


int arm (char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "stop", LINEBUFFER_MAX)) {
			set_Arm(ARM_STOP);
		} else if (!strncmp (args, "up", LINEBUFFER_MAX)) {
			set_Arm(ARM_UP);
		} else if (!strncmp (args, "down", LINEBUFFER_MAX)) {
			set_Arm(ARM_DOWN);
		} else
			/* Syntax error */
			return (-1);
	}

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

	return (result);
}


int dosage (char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "off", LINEBUFFER_MAX)) {
			set_Dosage(0);
		} else if (!strncmp (args, "on", LINEBUFFER_MAX)) {
			set_Dosage(1);
		} else
			/* Syntax error */
			return (-1);
	}

	printf("Dosage: %s\n", get_Dosage()?"on":"off");

	return (result);
}


int tap (char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "off", LINEBUFFER_MAX)) {
			set_Water(0);
		} else if (!strncmp (args, "on", LINEBUFFER_MAX)) {
			set_Water(1);
		} else
			/* Syntax error */
			return (-1);
	}

	printf("Tap: %s\n", get_Water()?"on":"off");

	return (result);
}


int drain (char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "off", LINEBUFFER_MAX)) {
			set_Pump(0);
		} else if (!strncmp (args, "on", LINEBUFFER_MAX)) {
			set_Pump(1);
		} else
			/* Syntax error */
			return (-1);
	}

	printf("Drain: %s\n", get_Pump()?"on":"off");

	return (result);
}


int dryer (char *args)
{
	int	result = 0;

	if (strlen(args)) {
		if (!strncmp (args, "off", LINEBUFFER_MAX)) {
			set_Dryer(0);
		} else if (!strncmp (args, "on", LINEBUFFER_MAX)) {
			set_Dryer(1);
		} else
			/* Syntax error */
			return (-1);
	}

	printf("Dryer: %s\n", get_Dryer()?"on":"off");

	return (result);
}


int cat (char *args)
{
	int	result = 0;

	if (strlen(args))
		/* Syntax error */
		return (-1);

	DBG("Cat: %s\n", cat_detected?"in":"out");

	return (result);
}


int water (char *args)
{
	int	result = 0;

	if (strlen(args))
		/* Syntax error */
		return (-1);

	printf("Water: %s\n", water_low?"low":"high");

	return (result);
}


int heat (char *args)
{
	int	result = 0;

	if (strlen(args))
		/* Syntax error */
		return (-1);

	printf("Overheat: %s\n", overheated?"yes":"no");

	return (result);
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
