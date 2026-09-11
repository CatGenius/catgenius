/******************************************************************************/
/* File    :	cmdline_box.c						      */
/* Function:	Command line commands operating the box			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	31 Mar 2012 by R. Delien:				      */
/*		- First creation					      */
/******************************************************************************/
#include <xc.h>
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
extern __bit	cat_detected;
extern __bit	overheated;

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

	printf("Cat: %s\n", cat_detected?"in":"out");

	return ERR_OK;
}


int water (int argc, char* argv[])
{
	if (argc > 1)
		return ERR_SYNTAX;

	printf("Water: %s\n", water_failed()?"sensor timeout":
		(water_valid()?(water_detected()?"high":"low"):"unqualified"));
#ifdef WATERSENSOR_ANALOG
	printf("Reflection ADC mean (last completed cycle): ");
	if (water_reflection_valid())
		printf("%u; fill: %s\n", water_reflectionquality(),
			water_reflection_filling()?str_on:str_off);
	else
		printf("unavailable\n");
	printf("Quality: ");
	switch (water_quality()) {
	case WATER_QUALITY_UNCHECKED:
		printf("unchecked");
		break;
	case WATER_QUALITY_CHECKING:
		printf("checking");
		break;
	case WATER_QUALITY_GOOD:
		printf("good");
		break;
	case WATER_QUALITY_OPTICAL:
		printf("poor optical reflection");
		break;
	case WATER_QUALITY_LEVEL:
		printf("high water or comparator fault");
		break;
	default:
		printf(str_unkown);
		break;
	}
	printf(" (threshold %u)\n", (unsigned int)WATER_QUALITY_THRESHOLD);
	printf("Acquisition: ");
	switch (water_acquisition_fault()) {
	case WATER_ACQUISITION_OK:
		printf("no timeout");
		break;
	case WATER_ACQUISITION_ADC:
		printf("ADC timeout");
		break;
	case WATER_ACQUISITION_PROBE:
		printf("probe timeout");
		break;
	default:
		printf(str_unkown);
		break;
	}
	printf("\nComparator RB3 (last enabled probe): ");
	if (water_comparator_valid())
		printf("%u\n", water_comparator());
	else
		printf("unavailable\n");
#else
	printf("Reflection digital (last completed): %u\n", water_reflectionquality());
#endif /* WATERSENSOR_ANALOG */
	printf("Fill requested: %s; RD0 now: %u; IR LED now: %s\n",
		water_filling()?str_on:str_off,
		(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK)?1:0,
		(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK)?str_on:str_off);
	printf("Comparator RB3 (instantaneous): %u%s\n",
		(WATERVALVE(PORT) & WATERVALVE_MASK)?1:0,
		(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK)?"":" (pull-up disabled)");

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
