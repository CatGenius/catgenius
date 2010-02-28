/******************************************************************************/
/* File    :	litterlanguage.c					      */
/* Function:	CatGenius LitterLanguage interpreter			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	26 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "litterlanguage.h"
#include "romwashprogram.h"
#include "../common/timer.h"
#include "../common/catgenie120.h"
#include "../common/hardware.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	prg_source	= 0;
static unsigned char	prg_scooponly	= 0;

static unsigned char	cmd_state	= 0;
static unsigned char	cmd_pointer	= 0;
static struct command	command;

static struct timer	timer_waitcmd   = NEVER;
static struct timer	timer_fill      = NEVER;
static struct timer	timer_drain     = NEVER;
static struct timer	timer_autodose  = NEVER;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		get_command (unsigned char cmd_pointer);
static unsigned char	got_command (struct command *command);
static void		exe_command (void);
static void		wait_command (void);


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void litterlanguage_init (void)
/******************************************************************************/
/* Function:	litterlanguage_init					      */
/*		- Initialize the CatGenius LitterLanguage interpreter	      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* litterlanguage_init */


void litterlanguage_work (void)
/******************************************************************************/
/* Function:	litterlanguage_work					      */
/*		- Worker function for the CatGenius LitterLanguage interpreter*/
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Check error timeouts */
	if (timeoutexpired(&timer_fill)) {
		/* Fill error */
		/* Pauze */
	}
	if (timeoutexpired(&timer_drain)) {
		/* Drain error */
		/* Pauze */
	}

	/* Check auto command timeouts */
	if (timeoutexpired(&timer_autodose)) {
		set_Dosage(0);
		timeoutnever(&timer_autodose);
	}

	switch (cmd_state) {
	case 0:	/* Idle */
		break;
	case 1:	/* Fetch a new command*/
		get_command(cmd_pointer);
		cmd_state ++;
		break;
	case 2:	/* Wait for the new command to be fetched */
		if (got_command(&command))
			cmd_state ++;
		break;
	case 3:	/* Decode and execute the command */
		exe_command();
		break;
	case 4:	/* Wait for the command to finish */
		wait_command();
		break;
	}
}
/* litterlanguage_work */


void litterlanguage_term (void)
/******************************************************************************/
/* Function:	litterlanguage_term					      */
/*		- Terminate the CatGenius LitterLanguage interpreter	      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* litterlanguage_term */


void litterlanguage_mode (unsigned char justscoop)
{
	if (!cmd_pointer)
		prg_scooponly = justscoop ;
}


void litterlanguage_start (void)
{
	if (!cmd_pointer) {
		cmd_pointer = 0;
		cmd_state ++ ;
	}
}


unsigned char litterlanguage_running (void)
{
	return (cmd_state);
}


void litterlanguage_pause (unsigned char pause)
{
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void get_command (unsigned char cmd_pointer)
{
	switch (prg_source) {
	case SRC_ROM:
		romwashprogram_getcmd(cmd_pointer);
	}
}

static unsigned char got_command (struct command *command)
{
	switch (prg_source) {
	default:
	case SRC_ROM:
		return romwashprogram_gotcmd(command);
	}
}

static void exe_command (void)
{
	switch (command.cmd) {
	case CMD_START:
		/* Check if this is a valid program for us */
		if( ((command.arg & 0x00FF) <= CMD_LAST) && 
		    ( (prg_scooponly && (command.arg & FLAGS_DRYRUN)) ||
		      (!prg_scooponly && (command.arg & FLAGS_WETRUN)) ) ) {
			cmd_pointer++;
			cmd_state -= 2;
		} else {
			cmd_state = 0;
		}
		break;
	case CMD_BOWL:
		set_Bowl((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_ARM:
		set_Arm((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WATER:
		if (!prg_scooponly) {
			set_Water((unsigned char)command.arg);
			if (command.arg)
				settimeout(&timer_fill, MAX_FILLTIME);
			else
				timeoutnever(&timer_fill);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DOSAGE:
		if (!prg_scooponly)
			set_Dosage((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_PUMP:
		if (!prg_scooponly) {
			set_Pump((unsigned char)command.arg);
			if (command.arg)
				settimeout(&timer_drain, MAX_DRAINTIME);
			else
				timeoutnever(&timer_drain);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DRYER:
		if (!prg_scooponly)
			set_Dryer((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WAITTIME:
		settimeout( &timer_waitcmd,
			    (unsigned long)command.arg * MILISECOND );
		cmd_state++;
		break;
	case CMD_WAITWATER:
		if (!prg_scooponly)
			cmd_state++;
		else {
			cmd_pointer++;
			cmd_state -= 2;
		}
		break;
	case CMD_SKIPIFDRY:
		if (prg_scooponly)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_SKIPIFWET:
		if (!prg_scooponly)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_AUTODOSE:
		if (!prg_scooponly) {
			settimeout(&timer_autodose, command.arg);
			set_Dosage(1);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_END:
		timeoutnever(&timer_fill);
		timeoutnever(&timer_drain);
		cmd_pointer = 0;
		cmd_state = 0;
		break;
	default:
		/* Program error */
		cmd_pointer = 0;
		break;
	}
}

static void wait_command (void)
{
	switch (command.cmd) {
	case CMD_WAITTIME:
		if (timeoutexpired(&timer_waitcmd)) {
			cmd_pointer++;
			cmd_state -= 3;
		}
		break;
	case CMD_WAITWATER:
		if (command.arg) {
			if (detected_Water()) {
				timeoutnever(&timer_fill);
				cmd_pointer++;
				cmd_state -= 3;
			}
		} else {
			if (!detected_Water()) {
				timeoutnever(&timer_drain);
				cmd_pointer++;
				cmd_state -= 3;
			}
		}
		break;
	default:
		cmd_pointer++;
		cmd_state -= 3;
		break;
	}
}