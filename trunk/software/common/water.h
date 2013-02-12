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
 * state, and the analog reflection quality value read by the ADC. A light guide
 * is considered to be dirty when the original firmware starts warning.
 * Cleanliness:	| State:	| Reflection:
 * -------------+---------------+--------------------
 * Clean	| Dry		| ~22
 * Clean	| Submerged	| 1023
 * Clean	| Wet		| ~22
 * Dirty	| Dry		| ~414
 * Dirty	| Submerged	| ~1011
 * Dirty	| Wet		| No reliable measurement yet
 
 */
#define GUIDEDIRTY_THRESHOLD	414		/* At an ADC value of 414 or above, the original firmware warns to clean the light guide */


/* Generic */
void		water_init		(void) ;
void		water_work		(void) ;

/* Getters */
unsigned char	water_detected		(void) ;
unsigned char	water_filling		(void) ;
/* Setters */
void		water_fill		(unsigned char fill) ;
void		water_ledalwayson	(unsigned char on) ;

#endif /* WATER_H */
