/******************************************************************************/
/* File    :	userinterface.h						      */
/* Function:	Definition of ports, pins and their functions.		      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef USERINTERFACE_H				/* Include file already compiled? */
#define USERINTERFACE_H


/* Generic */
void		userinterface_init	(void) ;
void		userinterface_work	(void) ;

/* External controls */
void		setup_short		(void) ;
void		setup_long		(void) ;
void		start_short		(void) ;
void		start_long		(void) ;
void		both_short		(void) ;
void		both_long		(void) ;

#endif /* USERINTERFACE_H */
