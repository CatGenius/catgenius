/******************************************************************************/
/* File    :	cr14.h							      */
/* Function:	Include file of 'cr14.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CR14_H					/* Include file already compiled? */
#define CR14_H

/* Generic */
void		cr14_init		(void) ;
void		cr14_work		(void) ;
void		cr14_term		(void) ;

unsigned char	cr14_busy		(void) ;
void		cr14_writeparamreg	(unsigned char	regval) ;

#endif /* CR14_H */
