/******************************************************************************/
/* File    :	watersensor.h						      */
/* Function:	Include file of 'watersensor.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef WATERSENSOR_H				/* Include file already compiled? */
#define WATERSENSOR_H

/* Generic */
void		watersensor_init	(void) ;
void		watersensor_work	(void) ;
void		watersensor_term	(void) ;

/* Getter */
unsigned char	watersensor_det		(void) ;

#endif /* WATERSENSOR_H */
