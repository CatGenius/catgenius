/******************************************************************************/
/* File    :	watersensor.c						      */
/* Function:	Water sensor functional implementation			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	2 Oct 2010 by R. Delien:				      */
/*		- Extracted from catgenie120.c.				      */
/******************************************************************************/
#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "watersensor.h"
#include "timer.h"

extern void watersensor_event (unsigned char detected);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define DETECTTIME		(SECOND/1000)	/*  10ms*/
#define WATERSENSORPOLLING	(SECOND/4)	/* 250ms*/
#define DECT_MARGIN		100		/* Margin to notify software of high water before the LM393 closes the water valve */
#define DECT_THRESHOLD		505		/* At an ADC value of 505 or below, the LM393 opens the water valve */
#define HYSTERESIS_MAX		8		/* Number of pollings to debounce the sensor output */

#define LED_ON			0
#define START_CONVERSION	1
#define PROCESS_RESULT		2


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	water_sensortimer = EXPIRED;
static unsigned char	water_state       = 0;
static unsigned char	water_hysteresis  = 0;
static unsigned int	water_sigquality  = 0;
static bit		water_filling     = 0;
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
#if (defined _16F877A)
	ADCON0bits.ADCS  = 3;
	ADCON1bits.ADCS2 = 1;
#elif (defined _16F1939)
	ADCON1bits.ADCS = 7;
#endif
	/* Set negative reference to Vss, positive reference to Vdd */
#if (defined _16F877A)
	ADCON1bits.PCFG = 0;
#elif (defined _16F1939)
	ADCON1bits.ADNREF = 0;
	ADCON1bits.ADPREF = 0;
#endif
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
	switch (water_state) {
	default:
		water_state = LED_ON;
	case LED_ON:
		if (!timeoutexpired(&water_sensortimer))
			break;
		/* Switch on the IR LED */
		WATERSENSOR_LED_PORT |= WATERSENSOR_LED_MASK;
		/* Wait for DETECTTIME to give the IR sensor some time */
		settimeout(&water_sensortimer, DETECTTIME);
		water_state = START_CONVERSION;
		break;
	case START_CONVERSION:
		if (!timeoutexpired(&water_sensortimer))
			break;
		/* Start A/D conversion */
		ADCON0bits.GO = 1;
		water_state = PROCESS_RESULT;
		break;
	case PROCESS_RESULT:
		if (ADCON0bits.nDONE)
			break;
		/* Read out the IR sensor (lower value == more light reflected == no water detected) */
		water_sigquality = (ADRESH << 8) | ADRESL;
		/* Switch off the IR LED if we're not filling */
		if (!water_filling)
			WATERSENSOR_LED_PORT &= ~WATERSENSOR_LED_MASK;
		/* Evaluate the result, considering a hysteresis */
		if (water_sigquality < (DECT_THRESHOLD - DECT_MARGIN)) {
			if ((water_hysteresis > 0) &&
			    (!--water_hysteresis && water_detected)) {
				water_detected = 0;
				watersensor_event(water_detected);
			}
		} else {
			if ((water_hysteresis < HYSTERESIS_MAX) &&
			    (++water_hysteresis >= HYSTERESIS_MAX) && !water_detected) {
				water_detected = 1;
				watersensor_event(water_detected);
			}
		}

		settimeout(&water_sensortimer, WATERSENSORPOLLING);
		water_state = LED_ON;
		break;
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


unsigned char watersensor_det (void)
{
	return (water_detected);
}
/* End: watersensor_det */


unsigned int watersensor_sigquality (void)
{
	return (water_sigquality);
}
/* End: watersensor_sigquality */


unsigned char get_Water (void)
{
	return (water_filling);
}
/* End: get_Water */


void set_Water (unsigned char on)
{
	water_filling = on;

	if (water_filling) {
		/* Pull-up WATERVALVE */
		WATERVALVEPULLUP_PORT |= WATERVALVEPULLUP_MASK;
	} else {
		/* Pull-down WATERVALVE */
		WATERVALVEPULLUP_PORT &= ~WATERVALVEPULLUP_MASK;
	}
}
/* End: set_Water */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
