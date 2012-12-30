/******************************************************************************/
/* File    :	water.h							      */
/* Function:	Include file of 'water.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef WATER_H					/* Include file already compiled? */
#define WATER_H

/* Generic */
void		watersensor_init	(void) ;
void		watersensor_work	(void) ;
void		watersensor_term	(void) ;

/* Getters */
unsigned int	watersensor_sigquality	(void) ;
unsigned char	watersensor_det		(void) ;
unsigned char	get_Water		(void) ;
/* Setters */
void		set_Water		(unsigned char on) ;
void		watersensor_ledalwayson	(unsigned char on) ;

#endif /* WATER_H */
