/******************************************************************************/
/* File    :	water.c							      */
/* Function:	Water valve and sensor functional implementation	      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	30 Dec 2012 by R. Delien:				      */
/*		- Renamed from watersensor.c.				      */
/******************************************************************************/
#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "water.h"
#include "timer.h"

extern void watersensor_event (unsigned char detected);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define DETECTTIME		(SECOND/1000)	/*  10ms*/
#define WATERSENSORPOLLING	(SECOND/4)	/* 250ms*/
#define HYSTERESIS_MAX		8		/* Number of pollings to debounce the sensor output */
#define DECT_MARGIN		100		/* Margin to notify software of high water before the LM393 closes the water valve */
#define DECT_THRESHOLD		505		/* At an ADC value of 505 or below, the LM393 opens the water valve */

#define LED_ON			0
#define START_CONVERSION	1
#define PROCESS_RESULT		2


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	sensortimer       = EXPIRED;
static unsigned char	state             = 0;
static unsigned char	hysteresis        = 0;
static unsigned int	reflectionquality = 0;
static bit		filling           = 0;
static bit		detected          = 0;
static bit		ledalwayson       = 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void water_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
#ifdef WATERSENSOR_ANALOG
	unsigned char	mask    = WATERSENSORANALOG_MASK;
	unsigned char	channel = 0;

	/* Dynamically determine channel# from mask */
	while (!(mask & 0x01)) {
		mask >>= 0x01;
		channel ++;
	}

	/* Power-up AD circuitry */
	ADCON0bits.ADON = 1;
	/* Select input channel */
	ADCON0bits.CHS = channel;

	/* Set output format to right-justified data */
	ADCON1bits.ADFM = 1;
	/* Set conversion clock to internal RC oscillator */
	ADCON1bits.ADCS = 7;

	/* Set negative reference to Vss, positive reference to Vdd */
	ADCON1bits.ADNREF = 0;
	ADCON1bits.ADPREF = 0;
#endif /* WATERSENSOR_ANALOG */
}
/* End: water_init */


void water_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		- Worker function for the CatGenie 120 water sensor and valve */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	switch (state) {
	default:
		state = LED_ON;
	case LED_ON:
		if (!timeoutexpired(&sensortimer))
			break;
		/* Switch on the IR LED */
		WATERSENSOR_LED(LAT) |= WATERSENSOR_LED_MASK;
		/* Wait for DETECTTIME to give the IR sensor some time */
		settimeout(&sensortimer, DETECTTIME);
#ifdef WATERSENSOR_ANALOG
		state = START_CONVERSION;
#else
		state = PROCESS_RESULT;
#endif /* WATERSENSOR_ANALOG */
		break;
	case START_CONVERSION:
		if (!timeoutexpired(&sensortimer))
			break;
		/* Start A/D conversion */
		ADCON0bits.GO = 1;
		state = PROCESS_RESULT;
		break;
	case PROCESS_RESULT:
#ifdef WATERSENSOR_ANALOG
		if (ADCON0bits.nDONE)
			break;
		/* Read out the IR sensor analoguely (lower value == more light reflected == no water detected) */
		reflectionquality = ADRES;
#else
		if (!timeoutexpired(&sensortimer))
			break;
		/* Read out the IR sensor digitally (lower value == more light reflected == no water detected) */
		reflectionquality = (WATERSENSORANALOG(PORT) & WATERSENSORANALOG_MASK)?DECT_THRESHOLD:0;
#endif /* WATERSENSOR_ANALOG */
		/* Switch off the IR LED if we're not filling */
		if (!filling && !ledalwayson)
			WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
		/* Evaluate the result, considering a hysteresis */
		if (reflectionquality <= (DECT_THRESHOLD - DECT_MARGIN)) {
			if ((hysteresis > 0) &&
			    (!--hysteresis && detected)) {
				detected = 0;
				watersensor_event(detected);
			}
		} else {
			if ((hysteresis < HYSTERESIS_MAX) &&
			    (++hysteresis >= HYSTERESIS_MAX) && !detected) {
				detected = 1;
				watersensor_event(detected);
			}
		}

		settimeout(&sensortimer, WATERSENSORPOLLING);
		state = LED_ON;
		break;
	}
}
/* End: water_work */


unsigned char water_detected (void)
{
	return (detected);
}
/* End: water_detected */


void water_ledalwayson (unsigned char on)
{
	ledalwayson = on;
}
/* End: water_ledalwayson */


unsigned int water_reflectionquality (void)
{
	return (reflectionquality);
}
/* End: water_reflectionquality */


unsigned char water_filling (void)
{
	return (filling);
}
/* End: water_filling */


void water_fill (unsigned char fill)
{
	filling = fill;

	if (filling) {
		/* Pull-up WATERVALVE */
		WATERVALVEPULLUP(LAT) |= WATERVALVEPULLUP_MASK;
	} else {
		/* Pull-down WATERVALVE */
		WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
	}
}
/* End: water_fill */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
