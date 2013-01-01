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

#include "../common/water.h"
#include "cmdline.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

/* Event statuses */
extern bit	cat_detected;
extern bit	overheated;

/* Frequently used strings declared once here, to save some memory */
char str_unkown[]	= "<unknown>";
char str_on[]		= "on";
char str_off[]		= "off";

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

int bowl(int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], "stop", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_STOP);
		} else if (!strncmp (argv[1], "cw", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_CW);
		} else if (!strncmp (argv[1], "ccw", LINEBUFFER_MAX)) {
			set_Bowl(BOWL_CCW);
		} else
			return ERR_SYNTAX;
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
		printf(str_unkown);
		break;
			
	}
	printf("\n");

	return ERR_OK;
}


int arm (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], "stop", LINEBUFFER_MAX)) {
			set_Arm(ARM_STOP);
		} else if (!strncmp (argv[1], "up", LINEBUFFER_MAX)) {
			set_Arm(ARM_UP);
		} else if (!strncmp (argv[1], "down", LINEBUFFER_MAX)) {
			set_Arm(ARM_DOWN);
		} else
			return ERR_SYNTAX;
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
		printf(str_unkown);
		break;
			
	}
	printf("\n");

	return ERR_OK;
}


int dosage (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], str_off, LINEBUFFER_MAX)) {
			set_Dosage(0);
		} else if (!strncmp (argv[1], str_on, LINEBUFFER_MAX)) {
			set_Dosage(1);
		} else
			return ERR_SYNTAX;
	}

	printf("Dosage: %s\n", get_Dosage()?str_on:str_off);

	return ERR_OK;
}


int tap (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], str_off, LINEBUFFER_MAX)) {
			water_fill(0);
		} else if (!strncmp (argv[1], str_on, LINEBUFFER_MAX)) {
			water_fill(1);
		} else
			return ERR_SYNTAX;
	}

	printf("Tap: %s\n", water_filling()?str_on:str_off);

	return ERR_OK;
}


int drain (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], str_off, LINEBUFFER_MAX)) {
			set_Pump(0);
		} else if (!strncmp (argv[1], str_on, LINEBUFFER_MAX)) {
			set_Pump(1);
		} else
			return ERR_SYNTAX;
	}

	printf("Drain: %s\n", get_Pump()?str_on:str_off);

	return ERR_OK;
}


int dryer (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc > 1) {
		if (!strncmp (argv[1], str_off, LINEBUFFER_MAX)) {
			set_Dryer(0);
		} else if (!strncmp (argv[1], str_on, LINEBUFFER_MAX)) {
			set_Dryer(1);
		} else
			return ERR_SYNTAX;
	}

	printf("Dryer: %s\n", get_Dryer()?str_on:str_off);

	return ERR_OK;
}


int cat (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	DBG("Cat: %s\n", cat_detected?"in":"out");

	return ERR_OK;
}


int water (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	printf("Water: %s\n", water_detected()?"high":"low");

	return ERR_OK;
}


int heat (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	printf("Overheat: %s\n", overheated?"yes":"no");

	return ERR_OK;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
