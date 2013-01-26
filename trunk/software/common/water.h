/******************************************************************************/
/* File    :	water.h							      */
/* Function:	Include file of 'water.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef WATER_H					/* Include file already compiled? */
#define WATER_H

/* Generic */
void		water_init		(void) ;
void		water_work		(void) ;

/* Getters */
unsigned int	water_reflectionquality	(void) ;
unsigned char	water_lightguidedirty	(void) ;
unsigned char	water_detected		(void) ;
unsigned char	water_filling		(void) ;
/* Setters */
void		water_fill		(unsigned char fill) ;
void		water_ledalwayson	(unsigned char on) ;

#endif /* WATER_H */
