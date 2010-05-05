/******************************************************************************/
/* File    :	litterlanguage.c					      */
/* Function:	CatGenius LitterLanguage interpreter			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	26 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "litterlanguage.h"
#include "romwashprogram.h"
#include "../common/timer.h"
#include "../common/catgenie120.h"
#include "../common/hardware.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BOX_TIDY	0
#define BOX_MESSY	1
#define BOX_WET		2


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

static void		litterlanguage_cleanup (unsigned char wet);
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
	DBG("Box is ");
	switch (eeprom_read(NVM_BOXSTATE)){
	case BOX_TIDY:
		DBG("tidy");
		break;
	case BOX_MESSY:
		DBG("messy");
		litterlanguage_cleanup(0);
		break;
	case BOX_WET:
		DBG("wet");
		litterlanguage_cleanup(1);
		break;
	default:
		DBG("unknown");
		eeprom_write(NVM_BOXSTATE, BOX_TIDY);
		break;
	}
	DBG("\n");
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
		DBG("Drain timout\n");
		timeoutnever(&timer_fill);
		/* Fill error */
		set_LED_Error(0x01, 1);
		set_Water(0);
		/* Pauze */
//		set_Bowl(BOWL_STOP);
//		set_Arm(ARM_STOP);
//		set_Dosage(0);
//		set_Pump(0);
//		set_Dryer(0);
//		set_Beeper(0x01, 1);
//		return;
	}
	if (timeoutexpired(&timer_drain)) {
		DBG("Drain timout\n");
		timeoutnever(&timer_drain);
		/* Drain error */
		set_LED_Error(0x05, 1);
		/* Pauze */
//		set_Bowl(BOWL_STOP);
//		set_Arm(ARM_STOP);
//		set_Water(0);
//		set_Dosage(0);
//		set_Pump(0);
//		set_Dryer(0);
//		set_Beeper(0x05, 1);
//		return;
	}

	/* Check auto command timeouts */
	if (timeoutexpired(&timer_autodose)) {
		DBG("AutoDose timout\n");
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
	if (!cmd_state) {
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

	DBG("Water %s\n", water_detected?"present":"gone");
	/* Disable proper timeout on event */
	if (water_detected) {
		/* Turn off the water and disable the timeout */
		set_Water(0);
		timeoutnever(&timer_fill);
	} else
		/* Just disable the timeout, we want to continue pumping */
		timeoutnever(&timer_drain);
}
/* watersensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void litterlanguage_cleanup (unsigned char wet)
{
	if (!cmd_state) {
		wet_program = wet;
		if (wet_program) {
			set_Bowl(BOWL_CW);
			cmd_pointer = 116;
		} else
			cmd_pointer = 37;
		cmd_state ++ ;
	}
}


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
	DBG("Line %d: ", cmd_pointer);
	switch (command.cmd) {
	case CMD_START:
		DBG("CMD_START, %s", wet_program?"wet":"dry");
		/* Check if this is a valid program for us */
		if( ((command.arg & 0x00FF) <= CMD_LAST) && 
		    ( (!wet_program && (command.arg & FLAGS_DRYRUN)) ||
		      (wet_program && (command.arg & FLAGS_WETRUN)) ) ) {
			eeprom_write(NVM_BOXSTATE, BOX_MESSY);
			cmd_pointer++;
			cmd_state -= 2;
		} else {
			cmd_state = 0;
			DBG(", failed");
		}
		break;
	case CMD_BOWL:
		DBG("CMD_BOWL, %s", (command.arg == BOWL_STOP)?"BOWL_STOP":((command.arg == BOWL_CW)?"BOWL_CW":"BOWL_CCW"));
		set_Bowl((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_ARM:
		DBG("CMD_ARM, %s", (command.arg == ARM_STOP)?"ARM_STOP":((command.arg == ARM_DOWN)?"ARM_DOWN":"ARM_UP"));
		set_Arm((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WATER:
		DBG("CMD_WATER, %s %s", command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			if (command.arg) {
				eeprom_write(NVM_BOXSTATE, BOX_WET);
				/* Don't fill if water is detected already */
				if (!water_detected) {
					set_Water(1);
					settimeout(&timer_fill, MAX_FILLTIME);
				} else
					DBG(" (skipped)");
			} else {
				/* Disable timeout on filling */
				set_Water(0);
				timeoutnever(&timer_fill);
			}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DOSAGE:
		DBG("CMD_DOSAGE, %s %s", command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Dosage((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_PUMP:
		DBG("CMD_PUMP, %s %s", command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program) {
			set_Pump((unsigned char)command.arg);
			if (command.arg)
				/* Only set timeout if water is still detected */
				if (water_detected)
					settimeout(&timer_drain, MAX_DRAINTIME);
			else
				timeoutnever(&timer_drain);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DRYER:
		DBG("CMD_DRYER, %s %s", command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Dryer((unsigned char)command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WAITTIME:
		DBG("CMD_WAITTIME, %u ms", command.arg);
		settimeout( &timer_waitcmd,
			    (unsigned long)command.arg * MILISECOND );
		cmd_state++;
		break;
	case CMD_WAITWATER:
		DBG("CMD_WAITWATER %s", wet_program?"":" (nop)");
		if (wet_program)
			cmd_state++;
		else {
			cmd_pointer++;
			cmd_state -= 2;
		}
		break;
	case CMD_SKIPIFDRY:
		DBG("CMD_SKIPIFDRY, %u %s", command.arg, wet_program?" (nop)":"");
		if (!wet_program)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_SKIPIFWET:
		DBG("CMD_SKIPIFWET, %u %s", command.arg, wet_program?"":" (nop)");
		if (wet_program)
			cmd_pointer += command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_AUTODOSE:
		DBG("CMD_AUTODOSE, %u ms %s", command.arg, wet_program?"":" (nop)");
		if (wet_program) {
			settimeout(&timer_autodose,
				   (unsigned long)command.arg * MILISECOND);
			set_Dosage(1);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_END:
		DBG("CMD_END");
		/* Disable timeouts */
		timeoutnever(&timer_fill);
		timeoutnever(&timer_drain);
		cmd_pointer = 0;
		cmd_state = 0;
		eeprom_write(NVM_BOXSTATE, BOX_TIDY);
		break;
	default:
		DBG("CMD_unknown: %d", command.cmd);
		/* Program error */
		cmd_pointer = 0;
		break;
	}
	DBG("\n");
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
				cmd_pointer++;
				cmd_state -= 3;
			}
		} else {
			if (!water_detected) {
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
