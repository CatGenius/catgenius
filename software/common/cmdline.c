/******************************************************************************/
/* File    :	cmdline.c						      */
/* Function:	Command line module					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/

#include "../common/app_prefs.h"

#ifdef HAS_COMMANDLINE

#include <htc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>				/* For atoi() */

#define CMDLINE_C
#include "cmdline.h"
#include "serial.h"

#include "../common/hardware.h"	/* For _XTAL_FREQ */
#include "timer.h"				/* For timing definitions/functions */

#ifdef APP_CATGENIUS
#include "../catgenius/userinterface.h"		/* For set_mode() */
#endif

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define PROMPT		"# "

#define ENQ		0x05
#define ACK		0x06

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

extern const struct 	command	commands[];

/******************************************************************************/
/* Local Data                                                                 */
/******************************************************************************/

static char				linebuffer[LINEBUFFER_MAX];
static unsigned char	localecho = 1;

/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void proc_char (char rxd);
static void proc_line (char *line);
static int cmd2index (char *cmd);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void cmdline_init (void)
/******************************************************************************/
/* Function:	Module initialization routine				      */
/*		- Initializes the command line interface		      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	if (localecho)
		TX(PROMPT);
}
/* End: cmdline_init */


void cmdline_work (void)
/******************************************************************************/
/* Function:	Module working routine					      */
/*		- Worker function for the command line interface	      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	char rxd ;

	while (readch(&rxd))
		switch(rxd) {
		case ENQ:
			putch(ACK);
			break;
		default:
			proc_char(rxd);
		}
}
/* End: cmdline_work */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void proc_char (char rxd)
{
	static unsigned char curcolumn = 0;

	if ((rxd >= ' ') && (rxd <= '~')) {
		if (curcolumn < (LINEBUFFER_MAX-1)) {
			/* Add readable characters to the line as long as the buffer permits */
			linebuffer[curcolumn] = rxd;
			curcolumn++;

			if (localecho)
				putch(rxd);
		} else
			/* Sound the bell is the buffer is full */
			if (localecho)
				putch('\a');
	} else if (rxd == '\r') {
		if (localecho)
			putch('\n');

		if (curcolumn) {
			/* Terminate string */
			linebuffer[curcolumn] = 0;
			/* Process string */
			proc_line(linebuffer);
		}
		curcolumn = 0;

		if (localecho)
			TX(PROMPT);
	} else if ((rxd == 0x7f) || (rxd == 0x08)) {
		/* Delete last character from the line */
		if (curcolumn) {
			/* Remove last character from the buffer */
			curcolumn--;

			if (localecho)
			{
				putch(0x08);
				putch(' ');
				putch(0x08);
			}
		}
	}
}


static void proc_line (char *line)
{
	unsigned int	len = strlen(line);
	int		argc = 0;
	char*		argv[ARGS_MAX];
	int		index;

	/* Trim trailing white spaces */
	while (len && (line[len-1] == ' ' || line[len-1] == '\t')) {
		line[len-1] = 0;
		len--;
	}

	/* Build argument list */
	while (*line) {
		/* Skip leading white spaces and replace them with 0-terminations */
		while ((*line == ' ') || (*line == '\t')) {
			*line = 0;
			line++;
		}

		/* Store the beginning of this argument */
		if (*line) {
			argv[argc] = line;
			argc++;
		}

		/* Find the separating white space */
		while (*line && *line != ' ' && *line != '\t')
			line++;
	}

	index = cmd2index(argv[0]);
	if (index >= 0) {
		switch (commands[index].function(argc, argv)) {
		case ERR_OK:
			break;
		case ERR_SYNTAX:
			TX("Syntax error\n");
			break;
		case ERR_IO:
			TX("I/O error\n");
			break;
		case ERR_PARAM:
			TX("Parameter error\n");
			break;
		default:
			TX("Unknown error\n");
		}
	} else
		TX2("Unknown command '%s'\n", argv[0]);
}


static int cmd2index (char *cmd)
{
	int index = 0;

	while (commands[index].function) {
		if (!strncmp (cmd, commands[index].cmd, COMMAND_MAX))
			return index;
		index++;
	}

	return (-1);
}


int cmd_echo (int argc, char* argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc == 2) {
		if (!strncmp (argv[1], "on", LINEBUFFER_MAX))
			localecho = 1;
		else if (!strncmp (argv[1], "off", LINEBUFFER_MAX))
			localecho = 0;
		else
			return ERR_SYNTAX;
	}
	TX2("Echo: %s\n", localecho?"on":"off");

	return ERR_OK;
}


int cmd_help (int argc, char* argv[])
{
	int index = 0;

	if (argc > 1)
		return ERR_SYNTAX;

	TX("Known commands:\n");
	while (commands[index].function) {
		TX2("%s\n", (char*)commands[index].cmd);
		index++;
	}

	return ERR_OK;
}

#ifdef HAS_COMMANDLINE_EXTRA

int cmd_mode (int argc, char* argv[])
{
	if (argc > 2) return ERR_SYNTAX;
	if (argc == 2) userinterface_set_mode((unsigned char)atoi(argv[1]));

	TX2("Mode: %d\n", auto_mode);

	return ERR_OK;
}

int cmd_start(int argc, char* argv[])
{
	unsigned char long_press = 0;

	if (argc > 2) return ERR_SYNTAX;
	if (argc == 2) long_press = stricmp(argv[1], "long") ? 0 : 1;

	if (long_press)
		start_long();
	else
		start_short();

	return ERR_OK;
}

int cmd_setup(int argc, char* argv[])
{
	unsigned char long_press = 0;

	if (argc > 2) return ERR_SYNTAX;
	if (argc == 2) long_press = stricmp(argv[1], "long") ? 0 : 1;

	if (long_press)
		setup_long();
	else
		setup_short();

	return ERR_OK;
}

int cmd_lock (int argc, char* argv[])
{
	unsigned char new_value;

	if (argc > 2) return ERR_SYNTAX;
	if (argc == 2) {
		new_value = stricmp(argv[1], "on") ? 0 : 1;
		if (locked != new_value) both_long();
		update_display();
	}

	TX2("Lock: %s\n", locked?"on":"off");

	return ERR_OK;
}

int cmd_cart (int argc, char* argv[])
{
	unsigned char new_value;

	if (argc != 1) return ERR_SYNTAX;

	both_short();

	TX("Cart: Showing level\n");

	return ERR_OK;
}

#endif // HAS_COMMANDLINE_EXTRA

#ifdef HAS_COMMANDLINE_COMTESTS
int cmd_rxtest (int argc, char* argv[])
{
	int byte_count;
	char ch;
	char echo_on = 0;
	struct timer start_time;
	struct timer stop_time;
	double run_time;

	switch (argc)
	{
		case 1:
			break;
		case 2:
			echo_on = (strcmp(argv[1], "1") == 0);
			break;
		default:
			return ERR_SYNTAX;
	}

	TX("WAIT ^Z\n");

	byte_count = 0;
	while (1)
	{
		if (readch(&ch)) {
			// Start timing after receiving the first char
			if (byte_count == 0) gettimestamp(&start_time);

			// Stop when we get ^Z
			if (ch == 0x1A) break;

			// Echo char if on & not XON/XOFF
			if ((echo_on) && (ch != XON) && (ch != XOFF)) putch(ch);

			byte_count++;
		}
	}
	gettimestamp(&stop_time);

	run_time = ((double)timestampdiff(&stop_time, &start_time)) / ((double)SECOND);
	TX4("\n%db %.2fs %.2fbps\n", byte_count, run_time, ((double)byte_count * 8) / run_time);

	return ERR_OK;
}

int cmd_txtest (int argc, char* argv[])
{
	int byte_count, i;
	char ch;
	struct timer start_time;
	struct timer stop_time;
	double run_time;

	switch (argc)
	{
		case 1:
			byte_count = 100;
			break;
		case 2:
			byte_count = atoi(argv[1]);
			break;
		default:
			return ERR_SYNTAX;
	}

	ch = '0';

	gettimestamp(&start_time);
	for (i=0; i<byte_count; i++)
	{
		if ((i % 50) == 0)
		{
			if (i>0) TX("\n");
			TX2("%05d ", i);
		}

		TX2("%c", ch);
		if (ch++ == '9') ch = '0';
	}
	gettimestamp(&stop_time);
	// timestampdiff() seems to be broken...
	//run_time = timestampdiff(&start_time, &stop_time);
	run_time = ((double)timestampdiff(&stop_time, &start_time)) / ((double)SECOND);

	byte_count += (byte_count / 50) * 7; // Compensate for line header and LF
	TX4("\n%db %.2fs %.2fbps\n", byte_count, run_time, ((double)byte_count * 8) / run_time);

	return ERR_OK;
}

#endif // HAS_COMMANDLINE_COMTESTS

#endif /* HAS_COMMANDLINE */
