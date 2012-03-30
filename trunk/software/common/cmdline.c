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

#include "cmdline.h"

/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define PROMPT		"# "
#define BUFFER_MAX	16

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

extern const struct command	commands[];
static char			linebuffer[BUFFER_MAX];
static char			localecho = 1;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void proc_char (char rxd);
static void proc_line (char *line);
static int cmd2index(char *cmd);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void cmdline_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
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
/* Function:	Module initialisation routine				      */
/*		- Worker function for the command line interface	      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	if (RCIF) {
		char rxd ;
		if ((OERR) || (FERR)) {
			TXEN = 0;
			CREN = 0;
			rxd = RCREG;
			CREN = 1;
			TXEN = 1;
		} else {
			rxd = RCREG;
			proc_char(rxd);
		}
	}
}
/* End: cmdline_work */


void cmdline_term (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Terminates the command line interface			      */
/* History :	13 Feb 2011 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
}
/* End: cmdline_term */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void proc_char (char rxd)
{
	static char curcolumn = 0;

	if ((rxd >= ' ') && (rxd <= '~')) {
		if (curcolumn < (BUFFER_MAX-1)) {
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
	char *cmd = line;
	char *arg;
	int index;

	/* Skip heading spaces */
	while (*cmd == ' ')
		cmd ++;

	/* Find the separating space */
	arg = cmd;
	while (*arg != ' ' && *arg != 0)
		arg++;

	/* Replace space(s) with 0-termination for command */
	while (*arg == ' ') {
		*arg = 0;
		arg++;
	}

	index = cmd2index(cmd);
	if (index >= 0) {
		if (commands[index].function(arg))
			printf("Syntax error\n");
	} else
		printf("Unknown command '%s'\n", cmd);
}


static int cmd2index(char *cmd)
{
	int index = 0;

	while (commands[index].function) {
		if (!strncmp (cmd, commands[index].cmd, COMMAND_MAX))
			return index;
		index++;
	}

	return (-1);
}


int echo(char *args)
{
	if (strlen(args)) {
		if (!strncmp (args, "on", BUFFER_MAX))
			localecho = 1;
		else if (!strncmp (args, "off", BUFFER_MAX))
			localecho = 0;
		else
			/* Syntax error */
			return (-1);
	}
	printf("Echo: %s\n", localecho?"on":"off");

	return (0);
}


int dumpports(char *args)
{
	printf("TRISB = 0x%02X, PORTB = 0x%02X\n", TRISB, PORTB);
	printf("TRISC = 0x%02X, PORTC = 0x%02X\n", TRISC, PORTC);
	printf("TRISD = 0x%02X, PORTD = 0x%02X\n", TRISD, PORTD);
	printf("TRISE = 0x%02X, PORTE = 0x%02X\n", TRISE, PORTE);

	return (0);
}

