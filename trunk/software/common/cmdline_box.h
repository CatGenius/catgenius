/******************************************************************************/
/* File    :	cmdline_tag.h						      */
/* Function:	Include file of 'cmdline_tag.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2012, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CMDLINE_BOX_H			/* Include file already compiled? */
#define CMDLINE_BOX_H

/* Command implementations */
int		bowl			(char	*args) ;
int		arm			(char	*args) ;
int		dosage			(char	*args) ;
int		tap			(char	*args) ;
int		drain			(char	*args) ;
int		dryer			(char	*args) ;
int		cat			(char	*args) ;
int		water			(char	*args) ;
int		heat			(char	*args) ;

#endif /* CMDLINE_BOX_H */
