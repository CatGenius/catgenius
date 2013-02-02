/******************************************************************************/
/* File    :	cmdline.c						      */
/* Function:	Command line module					      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "cmdline.h"
#include "serial.h"

#ifdef HAS_COMMANDLINE
/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define PROMPT		"# "

#define ENQ		0x05
#define ACK		0x06


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

extern const struct command	commands[];
static char			linebuffer[LINEBUFFER_MAX];
static unsigned char		localecho = 1;


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
		printf(PROMPT);
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
			printf(PROMPT);
	} else if (rxd == 0x7f) {
		/* Delete last character from the line */
		if (curcolumn) {
			/* Remove last character from the buffer */
			curcolumn--;

			if (localecho)
				putch(rxd);
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
			printf("Syntax error\n");
			break;
		case ERR_IO:
			printf("I/O error\n");
			break;
		case ERR_PARAM:
			printf("Parameter error\n");
			break;
		default:
			printf("Unknown error\n");
		}
	} else
		printf("Unknown command '%s'\n", argv[0]);
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


int echo (int argc, char* argv[])
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
	printf("Echo: %s\n", localecho?"on":"off");

	return ERR_OK;
}


int help (int argc, char* argv[])
{
	int index = 0;

	if (argc > 1)
		return ERR_SYNTAX;

	printf("Known commands:\n");
	while (commands[index].function) {
		printf("%s\n", (char*)commands[index].cmd);
		index++;
	}

	return ERR_OK;
}
#endif /* HAS_COMMANDLINE */
