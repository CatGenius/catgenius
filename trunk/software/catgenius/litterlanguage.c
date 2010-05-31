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
static struct command	const * cmd_pointer	= 0;
static struct command	cur_command;

static struct timer	timer_waitcmd   = NEVER;
static struct timer	timer_fill      = NEVER;
static struct timer	timer_drain     = NEVER;
static struct timer	timer_autodose  = NEVER;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		litterlanguage_cleanup (unsigned char wet);
static void		req_command (struct command	const *command);
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
		DBG("Fill timeout\n");
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
		DBG("Drain timeout\n");
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
		timeoutnever(&timer_autodose);
		set_Dosage(0);
	}

	switch (cmd_state) {
	case 0:	/* Idle */
		break;
	case 1:	/* Fetch a new command*/
		req_command(cmd_pointer);
		cmd_state ++;
		break;
	case 2:	/* Wait for the new command to be fetched */
		if (get_command(&cur_command))
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
	extern const struct command	washprogram[];
	if (!cmd_state) {
		switch (prg_source) {
		case SRC_ROM:
			cmd_pointer = &washprogram[0];
			break;
		}
		wet_program = wet;
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


void litterlanguage_stop (void)
{
	if (cmd_state) {
		/* Stop all actuators */
		set_Bowl(BOWL_STOP);
		set_Arm(ARM_STOP);
		set_Water(0);
		set_Dosage(0);
		set_Pump(0);
		set_Dryer(0);
		/* Disable timeouts */
		timeoutnever(&timer_fill);
		timeoutnever(&timer_drain);
		timeoutnever(&timer_autodose);
		cmd_state = 0;
	}
}


void watersensor_event (unsigned char undetected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	water_detected = !undetected;

	DBG("Water %s\n", water_detected?"high":"low");
	if (cmd_state) {
		/* Disable timeout on event */
		if (water_detected) {
			/* Turn off the water and disable the timeout */
			set_Water(0);
			timeoutnever(&timer_fill);
		}
	} else
		if (water_detected)
			DBG("Box flooded!\n");
}
/* watersensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void litterlanguage_cleanup (unsigned char wet)
{
	extern const struct command	cleanupprogram[];
	if (!cmd_state) {
		prg_source = SRC_ROM;
		cmd_pointer = &cleanupprogram[0];
		wet_program = wet;
		cmd_state ++ ;
	}
}


static void req_command (struct command const *command)
{
	switch (prg_source) {
	case SRC_ROM:
		romwashprogram_reqcmd(command);
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
	static struct command	const * ret_address;

	DBG("IP 0x%04X: ", cmd_pointer);
	switch (cur_command.cmd) {
	case CMD_START:
		DBG("CMD_START, %s", wet_program?"wet":"dry");
		/* Check if this is a valid program for us */
		if( ((cur_command.arg & 0x00FF) <= CMD_END) && 
		    ( (!wet_program && (cur_command.arg & FLAGS_DRYRUN)) ||
		      (wet_program && (cur_command.arg & FLAGS_WETRUN)) ) ) {
			eeprom_write(NVM_BOXSTATE, BOX_MESSY);
			cmd_pointer++;
			cmd_state -= 2;
		} else {
			cmd_state = 0;
			DBG(", failed");
		}
		break;
	case CMD_BOWL:
		DBG("CMD_BOWL, %s", (cur_command.arg == BOWL_STOP)?"BOWL_STOP":((cur_command.arg == BOWL_CW)?"BOWL_CW":"BOWL_CCW"));
		set_Bowl((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_ARM:
		DBG("CMD_ARM, %s", (cur_command.arg == ARM_STOP)?"ARM_STOP":((cur_command.arg == ARM_DOWN)?"ARM_DOWN":"ARM_UP"));
		set_Arm((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WATER:
		DBG("CMD_WATER, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			if (cur_command.arg) {
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
	case CMD_PUMP:
		DBG("CMD_PUMP, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Pump((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_DRYER:
		DBG("CMD_DRYER, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Dryer((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_WAITTIME:
		DBG("CMD_WAITTIME, %ums", cur_command.arg);
		settimeout( &timer_waitcmd,
			    (unsigned long)cur_command.arg * MILISECOND );
		cmd_state++;
		break;
	case CMD_WAITWATER:
		DBG("CMD_WAITWATER, %s%s", cur_command.arg?"high":"low", wet_program?"":" (nop)");
		if (wet_program) {
			if (!cur_command.arg)
				/* Start the drain timeout, we don't want to wait forever */
				settimeout(&timer_drain, MAX_DRAINTIME);
			cmd_state++;
		} else {
			cmd_pointer++;
			cmd_state -= 2;
		}
		break;
	case CMD_WAITDOSAGE:
		DBG("CMD_WAITDOSAGE%s", wet_program?"":" (nop)");
		if (wet_program)
			cmd_state++;
		else {
			cmd_pointer++;
			cmd_state -= 2;
		}
		break;
	case CMD_SKIPIFDRY:
		DBG("CMD_SKIPIFDRY, %u%s", cur_command.arg, wet_program?" (nop)":"");
		if (!wet_program)
			cmd_pointer += cur_command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_SKIPIFWET:
		DBG("CMD_SKIPIFWET, %u%s", cur_command.arg, wet_program?"":" (nop)");
		if (wet_program)
			cmd_pointer += cur_command.arg + 1;
		else
			cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_AUTODOSE:
		DBG("CMD_AUTODOSE, %u.%uml%s", cur_command.arg/10, cur_command.arg%10, wet_program?"":" (nop)");
		if (wet_program) {
			settimeout(&timer_autodose,
				   (unsigned long)cur_command.arg * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
			set_Dosage(1);
		}
		cmd_pointer++;
		cmd_state -= 2;
		break;
	case CMD_CALL:
		DBG("CMD_CALL, 0x%04X", cur_command.arg);
		ret_address = cmd_pointer + 1;
		cmd_pointer = (struct command const*)cur_command.arg;
		cmd_state -= 2;
		break;
	case CMD_RETURN:
		DBG("CMD_RETURN, 0x%04X", ret_address);
		cmd_pointer = ret_address;
		cmd_state -= 2;
		break;
	case CMD_END:
		DBG("CMD_END");
		eeprom_write(NVM_BOXSTATE, BOX_TIDY);
		litterlanguage_stop();
		break;
	default:
		/* Program error */
		DBG("CMD_unknown: 0x%X", cur_command.arg);
		litterlanguage_stop();
		break;
	}
	DBG("\n");
}

static void wait_command (void)
{
	switch (cur_command.cmd) {
	case CMD_WAITTIME:
		if (timeoutexpired(&timer_waitcmd)) {
			cmd_pointer++;
			cmd_state -= 3;
		}
		break;
	case CMD_WAITWATER:
		if (cur_command.arg) {
			if (water_detected) {
				cmd_pointer++;
				cmd_state -= 3;
			}
		} else {
			if (!water_detected) {
				/* Disable the timeout */
				timeoutnever(&timer_drain);
				cmd_pointer++;
				cmd_state -= 3;
			}
		}
		break;
	case CMD_WAITDOSAGE:
		if (!get_Dosage()) {
			cmd_pointer++;
			cmd_state -= 3;
		}
		break;
	default:
		cmd_pointer++;
		cmd_state -= 3;
		break;
	}
}
