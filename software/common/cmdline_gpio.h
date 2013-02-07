/******************************************************************************/
/* File    :	cmdline_gpio.h						      */
/* Function:	Include file of 'cmdline_gpio.c'.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2012, Clockwork Engineering		      */
/******************************************************************************/

#include "../common/app_prefs.h"

#if !(defined CMDLINE_GPIO_H) && (defined HAS_COMMANDLINE_GPIO)
#define CMDLINE_GPIO_H

/* Command implementations */
int		cmd_gpio		(int argc,	char* argv[]) ;

#endif // !CMDLINE_GPIO_H
