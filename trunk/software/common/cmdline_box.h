/******************************************************************************/
/* File    :	cmdline_box.h						      */
/* Function:	Include file of 'cmdline_box.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2012, Clockwork Engineering		      */
/******************************************************************************/

#include "../common/app_prefs.h"

#if !(defined CMDLINE_BOX_H) && (defined HAS_COMMANDLINE_BOX) /* Include file already compiled? */
#define CMDLINE_BOX_H

/* Command implementations */
int		cmd_bowl		(int argc,	char* argv[]) ;
int		cmd_arm			(int argc,	char* argv[]) ;
int		cmd_dosage		(int argc,	char* argv[]) ;
int		cmd_tap			(int argc,	char* argv[]) ;
int		cmd_drain		(int argc,	char* argv[]) ;
int		cmd_dryer		(int argc,	char* argv[]) ;
int		cmd_cat			(int argc,	char* argv[]) ;
int		cmd_water		(int argc,	char* argv[]) ;
int		cmd_heat		(int argc,	char* argv[]) ;

#endif // !CMDLINE_BOX_H