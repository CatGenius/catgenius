/******************************************************************************/
/* File    :	cmdline.h						      */
/* Function:	Include file of 'cmdline.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2011, Clockwork Engineering		      */
/******************************************************************************/
#ifndef CMDLINE_H			/* Include file already compiled? */
#define CMDLINE_H


#define LINEBUFFER_MAX	(20)	/* Maximum length of a complete command line */
#define COMMAND_MAX	(8)	/* Maximum length of a command name */
#define ARGS_MAX	(4)	/* Maximum number of arguments, including command */

#define ERR_OK		(0)
#define ERR_SYNTAX	(-1)
#define ERR_IO		(-2)
#define ERR_PARAM	(-3)

struct command {
	char	cmd[COMMAND_MAX];
	int	(*function)(int argc, char* argv[]);
};

/* Generic */
void		cmdline_init		(void) ;
void		cmdline_work		(void) ;
void		cmdline_term		(void) ;

/* Command implementations */
int		echo			(int argc,	char* argv[]) ;
int		help			(int argc,	char* argv[]) ;

#endif /* CMDLINE_H */
