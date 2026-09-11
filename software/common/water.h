/******************************************************************************/
/* File    :	water.h							      */
/* Function:	Include file of 'water.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef WATER_H					/* Include file already compiled? */
#define WATER_H


/*
 * The table below shows the correlation between light guide cleanliness and
 * state, and the analog reflection quality value read by the ADC. These are
 * observations, not sufficient by themselves to diagnose an optical fault.
 * Cleanliness:	| State:	| Reflection:
 * -------------+---------------+--------------------
 * Clean	| Dry		| ~22
 * Clean	| Submerged	| 1023
 * Clean	| Wet		| ~22
 * Dirty	| Dry		| ~414
 * Dirty	| Submerged	| ~1011
 * Dirty	| Wet		| No reliable measurement yet
 
 */
#define GUIDEDIRTY_THRESHOLD	414		/* Observed dirty/dry reading, not a level threshold */


/* Generic */
void		water_init		(void) ;
void		water_work		(void) ;

/* Getters */
unsigned char	water_detected		(void) ;
unsigned char	water_filling		(void) ;
unsigned char	water_valid		(void) ;	/* Debounced level is qualified */
unsigned char	water_failed		(void) ;	/* Acquisition timed out; clears after qualification */
unsigned int	water_reflectionquality	(void) ;	/* Last four-read mean; digital sample on 16F877A */
/* Setters */
void		water_fill		(unsigned char fill) ;
void		water_ledalwayson	(unsigned char on) ;

#ifdef WATERSENSOR_ANALOG
#include "waterquality.h"

#define WATER_ACQUISITION_OK	0
#define WATER_ACQUISITION_ADC	1
#define WATER_ACQUISITION_PROBE	2

/* Timer4 ISR: sample the comparator and restore outputs without callbacks. */
void		water_isr		(void) ;
/* Checks require stopped actuators. Optical faults cannot bypass recovery. */
unsigned char	water_check		(unsigned char probe) ;
void		water_check_cancel	(void) ;
unsigned char	water_quality		(void) ;
unsigned char	water_acquisition_fault	(void) ;
unsigned char	water_comparator	(void) ;
unsigned char	water_comparator_valid	(void) ;
unsigned char	water_reflection_filling(void) ;
unsigned char	water_reflection_valid	(void) ;	/* At least one complete cycle is available */
#endif

#endif /* WATER_H */
