/******************************************************************************/
/* File    :	cmdline_tag.h						      */
/* Function:	Include file of 'cmdline_tag.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2012, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CMDLINE_BOX_H			/* Include file already compiled? */
#define CMDLINE_BOX_H

/* Command implementations */
int		bowl			(int argc,	char* argv[]) ;
int		arm			(int argc,	char* argv[]) ;
int		dosage			(int argc,	char* argv[]) ;
int		tap			(int argc,	char* argv[]) ;
int		drain			(int argc,	char* argv[]) ;
int		dryer			(int argc,	char* argv[]) ;
int		cat			(int argc,	char* argv[]) ;
int		water			(int argc,	char* argv[]) ;
int		heat			(int argc,	char* argv[]) ;

#endif /* CMDLINE_BOX_H */
