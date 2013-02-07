/******************************************************************************/
/* File    :	cmdline_tag.h						      */
/* Function:	Include file of 'cmdline_tag.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2012, Clockwork Engineering		      */
/******************************************************************************/

#include "../common/app_prefs.h"

#if !(defined CMDLINE_TAG_H) && (defined HAS_COMMANDLINE_TAG)
#define CMDLINE_TAG_H

/* Command implementations */
int		cmd_tag			(int argc,	char* argv[]) ;

#endif // !CMDLINE_TAG_H
