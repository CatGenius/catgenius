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
static bit		wet_program	= 0;
static bit		water_detected	= 0;

static unsigned char	cmd_state	= 0;
static unsigned int	cmd_pointer	= 0;
static struct command	command;

static struct timer	timer_waitcmd   = NEVER;
static struct timer	timer_fill      = NEVER;
static struct timer	timer_drain     = NEVER;
static struct timer	timer_autodose  = NEVER;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		req_command (unsigned int	cmd_pointer);
static unsigned char	get_command (struct command	*command);
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
//		set_Bowl(BOWL_STOP);
//		set_Arm(ARM_STOP);
//		set_Water(0);
//		set_Dosage(0);
//		set_Pump(0);
//		set_Dryer(0);
//		set_Beeper(0x01, 1);
//		set_LED_Error(0x01, 1);
//		return;
	}
	if (timeoutexpired(&timer_drain)) {
		/* Drain error */
		/* Pauze */
//		set_Bowl(BOWL_STOP);
//		set_Arm(ARM_STOP);
//		set_Water(0);
//		set_Dosage(0);
//		set_Pump(0);
//		set_Dryer(0);
//		set_Beeper(0x05, 1);
//		set_LED_Error(0x05, 1);
//		return;
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
		req_command(cmd_pointer);
		cmd_state ++;
		break;
	case 2:	/* Wait for the new command to be fetched */
		if (get_command(&command))
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


void litterlanguage_start (unsigned char wet)
{
	if (!cmd_pointer) {
		wet_program = wet;
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


void watersensor_event (unsigned char undetected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	water_detected = ! undetected;

	/* Disable proper timeout on event */
	if (water_detected)
		timeoutnever(&timer_fill);
	else
		timeoutnever(&timer_drain);
}
/* watersensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void req_command (unsigned int cmd_pointer)
{
	switch (prg_source) {
	case SRC_ROM:
		romwashprogram_reqcmd(cmd_pointer);
	}
}

static unsigned char get_command (struct command *command)
{
	switch (prg_source) {
	default:
	case SRC_ROM:
		return romwashprogram_getcmd(command);
	}
}

static void exe_command (void)
{
	switch (command.cmd) {
	case CMD_START:
		/* Check if this is a valid program for us */
		if( ((command.arg & 0x00FF) <= CMD_LAST) && 
		    ( (!wet_program && (command.arg & FLAGS_DRYRUN)) ||
		      (wet_program && (command.arg & FLAGS_WETRUN)) ) ) {
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
		if (wet_program) {
			set_Water((unsigned char)command.arg);
			if (command.arg)
				settimeout(&timer_fill, MAX_FILLTIME);
			else
				/* Disable timeout on cancelation */
				timeoutnever(&timer_fill);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DOSAGE:
		if (wet_program)
			set_Dosage((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_PUMP:
		if (wet_program) {
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
		if (wet_program)
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
		if (wet_program)
			cmd_state++;
		else {
			cmd_pointer++;
			cmd_state -= 2;
		}
		break;
	case CMD_SKIPIFDRY:
		if (!wet_program)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_SKIPIFWET:
		if (wet_program)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_AUTODOSE:
		if (wet_program) {
			settimeout(&timer_autodose, command.arg);
			set_Dosage(1);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_END:
		/* Disable timeouts */
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
			if (water_detected) {
				/* Disable timeout on poll (event may not have occured again) */
				timeoutnever(&timer_fill);
				cmd_pointer++;
				cmd_state -= 3;
			}
		} else {
			if (!water_detected) {
				/* Disable timeout on poll (event may not have occured again) */
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
