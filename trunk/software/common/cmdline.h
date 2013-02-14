/******************************************************************************/
/* File    :	cmdline.h						      */
/* Function:	Include file of 'cmdline.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2011, Clockwork Engineering		      */
/******************************************************************************/

#include "../common/app_prefs.h"

#if !(defined CMDLINE_H) && (defined HAS_COMMANDLINE)
#define CMDLINE_H

#ifdef CMDLINE_C
#  include "../common/prot_src.h"
#else
#  include "../common/prot_inc.h"
#endif

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
PUBLIC_FN(void cmdline_init (void));
PUBLIC_FN(void cmdline_work (void));

/* Command implementations */
PUBLIC_FN(int cmd_echo   (int argc, char* argv[]));
PUBLIC_FN(int cmd_help   (int argc,char* argv[]));

#ifdef HAS_COMMANDLINE_COMTESTS
PUBLIC_FN(int cmd_txtest (int argc, char* argv[]));
PUBLIC_FN(int cmd_rxtest (int argc, char* argv[]));
#endif
#ifdef HAS_COMMANDLINE_EXTRA
PUBLIC_FN(int cmd_mode   (int argc, char* argv[]));
PUBLIC_FN(int cmd_start  (int argc, char* argv[]));
PUBLIC_FN(int cmd_setup  (int argc, char* argv[]));
PUBLIC_FN(int cmd_lock   (int argc, char* argv[]));
PUBLIC_FN(int cmd_cart   (int argc, char* argv[]));
#endif

#endif // !CMDLINE_H