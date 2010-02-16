/******************************************************************************/
/* File    :	catsensor.h						      */
/* Function:	Include file of 'catsensor.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CATSENSOR_H				/* Include file already compiled? */
#define CATSENSOR_H

void		catsensor_init		(void) ;
void		catsensor_work		(void) ;
void		catsensor_isr		(void) ;
void		catsensro_term		(void) ;

#endif /* CATSENSOR_H */
