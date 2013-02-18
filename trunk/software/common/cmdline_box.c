/******************************************************************************/
/* File    :	cmdline_box.c						      */
/* Function:	Command line commands operating the box			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	31 Mar 2012 by R. Delien:				      */
/*		- First creation					      */
/******************************************************************************/

#include "../common/app_prefs.h"

#ifdef HAS_COMMANDLINE_BOX

#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */
#include "../common/water.h"
#include "../common/serial.h"
#include "cmdline.h"

#ifdef APP_CATGENIUS
#include "../catgenius/userinterface.h"			/* For cat_detected */
#endif

#include "../common/timer.h"

#ifdef CMM_ARM_EXPERIMENT
#include <stdlib.h>				/* For atoi() */
#include "../catgenius/litterlanguage.h"
#endif


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

/* Event statuses */
//extern bit	cat_detected;
//extern bit	overheated;

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

int cmd_bowl(int argc, char* argv[])
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

	TX("Bowl: ");
	switch (get_Bowl()) {
	case BOWL_STOP:
		TX("stop");
		break;
	case BOWL_CW:
		TX("cw");
		break;
	case BOWL_CCW:
		TX("ccw");
		break;
	default:
		TX(str_unkown);
		break;
			
	}
	TX("\n");

	return ERR_OK;
}


int cmd_arm (int argc, char* argv[])
{
#ifdef CMM_ARM_EXPERIMENT
	unsigned char mode;
	unsigned long pos;
	unsigned char is_numeric;

	if (argc > 2)
		return ERR_SYNTAX;

	get_ArmPosition(&pos, &mode);
	TX("Arm: ");
	switch (mode) {
	case ARM_STOP:
		TX("stop");
		break;
	case ARM_UP:
		TX("up");
		break;
	case ARM_DOWN:
		TX("down");
		break;
	default:
		TX(str_unkown);
		break;
			
	}
	TX3(" %lu (%lu)\n", pos, pos / ARM_STROKE_PCT);

	if (argc > 1) {
		if (!strncmp (argv[1], "stop", LINEBUFFER_MAX)) {
			is_numeric = 0;
			mode = INS_ARM__STOP;
		} else if (!strncmp (argv[1], "up", LINEBUFFER_MAX)) {
			is_numeric = 0;
			mode = INS_ARM__UP;
		} else if (!strncmp (argv[1], "down", LINEBUFFER_MAX)) {
			is_numeric = 0;
			mode = INS_ARM__DOWN;
		} else if (!strncmp (argv[1], "0", LINEBUFFER_MAX)) {
			is_numeric = 1;
			mode = 0;
		} else {
			// If atoi() returns 0 but the string is not "0", then there was an error
			is_numeric = 1;
			mode = (unsigned char)atoi(argv[1]);
			if (mode == 0) return ERR_SYNTAX;
		}

		if (is_numeric)
			TX2("Arm target: %u\n", mode);
		else
			TX2("Arm target: %s\n", argv[1]);
		ins_Arm(mode);
	}

	return ERR_OK;

#else
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

	TX("Arm: ");
	switch (get_Arm()) {
	case ARM_STOP:
		TX("stop");
		break;
	case ARM_UP:
		TX("up");
		break;
	case ARM_DOWN:
		TX("down");
		break;
	default:
		TX(str_unkown);
		break;
			
	}
	TX("\n");

	return ERR_OK;
#endif
}


int cmd_dosage (int argc, char* argv[])
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

	TX2("Dosage: %s\n", get_Dosage()?str_on:str_off);

	return ERR_OK;
}


int cmd_tap (int argc, char* argv[])
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

	TX2("Tap: %s\n", water_filling()?str_on:str_off);

	return ERR_OK;
}


int cmd_drain (int argc, char* argv[])
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

	TX2("Drain: %s\n", get_Pump()?str_on:str_off);

	return ERR_OK;
}


int cmd_dryer (int argc, char* argv[])
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

	TX2("Dryer: %s\n", get_Dryer()?str_on:str_off);

	return ERR_OK;
}

int cmd_cat (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	TX2("Cat: %s\n", cat_detected?"in":"out");

	return ERR_OK;
}

int cmd_water (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	TX2("Water: %s\n", water_detected()?"high":"low");

	return ERR_OK;
}

int cmd_heat (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	TX2("Overheat: %s\n", error_overheat?"yes":"no");

	return ERR_OK;
}

/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

#endif // HAS_COMMANDLINE_BOX