/******************************************************************************/
/* File    :	catsensor.h						      */
/* Function:	Include file of 'catsensor.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATSENSOR_H				/* Include file already compiled? */
#define CATSENSOR_H

/* Generic */
void		catsensor_init		(void) ;
void		catsensor_work		(void) ;
void		catsensro_term		(void) ;

/* Event notification */
void		catsensor_isr_timer	(void) ;
void		catsensor_isr_input	(void) ;

#endif /* CATSENSOR_H */
