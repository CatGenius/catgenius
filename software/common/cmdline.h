/******************************************************************************/
/* File    :	cmdline.h						      */
/* Function:	Include file of 'cmdline.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2011, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CMDLINE_H			/* Include file already compiled? */
#define CMDLINE_H

#define LINEBUFFER_MAX	16	/* Maximum length of a complete command line */
#define COMMAND_MAX	8	/* Maximum length of a command name */

struct command {
	char	cmd[COMMAND_MAX];
	int	(*function)(char *args);
};

/* Generic */
void		cmdline_init		(void) ;
void		cmdline_work		(void) ;
void		cmdline_term		(void) ;

/* Command implementations */
int		echo			(char	*args) ;
int		dumpports		(char	*args) ;

#endif /* CMDLINE_H */
