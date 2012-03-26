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
#include <string.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "litterlanguage.h"
#include "romwashprogram.h"
#include "../common/timer.h"
#include "../common/rtc.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define BOX_TIDY		0
#define BOX_MESSY		1
#define BOX_WET			2

#define STATE_IDLE		0
#define STATE_FETCH_START	1
#define STATE_GET_START		2
#define STATE_FETCH_CMD		3
#define STATE_GET_CMD		4
#define STATE_WAIT_CMD		5

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char	prg_source		= 0;
static bit		wet_program		= 0;
static bit		water_detected		= 0;
static bit		overheat_detected	= 0;
static bit		paused			= 0;
static bit		error_fill		= 0;
static bit		error_drain		= 0;
static bit		error_flood		= 0;
static bit		error_dryer		= 0;

/* Program execution variables */
static unsigned char	cmd_state		= STATE_IDLE;
static struct command	const * cmd_pointer	= 0;
static struct command	cur_command;

static struct timer	timer_waitcmd   	= NEVER;
static struct timer	timer_fill      	= NEVER;
static struct timer	timer_drain     	= NEVER;
static struct timer	timer_autodose  	= NEVER;


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

void litterlanguage_init (unsigned char flags)
/******************************************************************************/
/* Function:	litterlanguage_init					      */
/*		- Initialize the CatGenius LitterLanguage interpreter	      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	switch(flags & BUTTONS) {
		case 0:
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
			break;
		case START_BUTTON_HELD:
			/* User wants to force a wet cleanup cycle */
			DBG("Wet cleanup forced\n");
			litterlanguage_cleanup(1);
			break;
		case SETUP_BUTTON_HELD:
			/* User wants to use GenieDiag */
			break;
		case START_BUTTON_HELD | SETUP_BUTTON_HELD:
		default:
			/* User wants to reset box state */
			eeprom_write(NVM_BOXSTATE, BOX_TIDY);
			break;
	}
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
	if( (cmd_state != STATE_IDLE) &&
	    !paused ){
		/* Check for filling timeout */
		if( !water_detected &&
		    get_Water() &&
		    timeoutexpired(&timer_fill) ){
			printtime();
			DBG("Fill timeout\n");
			timeoutnever(&timer_fill);
			/* Fill error */
			error_fill = 1;
			/* Pauze */
			litterlanguage_pause(1);
		}

		/* Check for draining timeout */
		if( water_detected &&
		    get_Pump() &&
		    timeoutexpired(&timer_drain) ){
			printtime();
			DBG("Drain timeout\n");
			timeoutnever(&timer_drain);
			/* Drain error */
			error_drain = 1;
			/* Pauze */
			litterlanguage_pause(1);
		}
		/* Check for overheating error */
		if( overheat_detected &&
		    get_Dryer() ){
			printtime();
			DBG("Overheating\n");
			/* Overheat error */
			error_dryer = 1;
			/* Pauze */
			litterlanguage_pause(1);
		}
	}

	/* Check auto command timeouts */
	if (timeoutexpired(&timer_autodose)) {
		timeoutnever(&timer_autodose);
		set_Dosage(0);
	}

	switch (cmd_state) {
	case STATE_IDLE:	/* Idle */
		break;

	case STATE_FETCH_START:	/* Fetch the start command*/
		req_command(cmd_pointer);
		cmd_state = STATE_GET_START;
		/* no break; */

	case STATE_GET_START:	/* Wait for the start command to be fetched */
		if (get_command(&cur_command)) {
			printtime();
			DBG("IP 0x%04X: ", cmd_pointer);
			if (cur_command.cmd == CMD_START) {
				DBG("CMD_START, %s", wet_program?"wet":"dry");
				/* Check if this is a valid program for us */
				if( ((cur_command.arg & 0x00FF) <= CMD_END) &&
				    ( (!wet_program && (cur_command.arg & FLAGS_DRYRUN)) ||
				      (wet_program && (cur_command.arg & FLAGS_WETRUN)) ) ) {
				      	if (eeprom_read(NVM_BOXSTATE) < BOX_MESSY)
						eeprom_write(NVM_BOXSTATE, BOX_MESSY);
					cmd_pointer++;
					cmd_state = STATE_FETCH_CMD;
				} else {
					cmd_state = STATE_IDLE;
					DBG(", incompatible");
				}
			} else {
				cmd_state = STATE_IDLE;
				DBG(", no start: 0x%X", cur_command.cmd);
			}
			DBG("\n");
		}
		break;

	case STATE_FETCH_CMD:	/* Fetch the next command*/
		req_command(cmd_pointer);
		cmd_state = STATE_GET_CMD;
		/* no break; */

	case STATE_GET_CMD:	/* Wait for the start command to be fetched */
		if (get_command(&cur_command)) {
			/* Decode and execute the command */
			exe_command();
		}
		break;

	case STATE_WAIT_CMD:	/* Wait for the command to finish */
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
	if (cmd_state == STATE_IDLE) {
		/* TODO: Stack overflow if we're printing here */
//		DBG("Starting %s program\n", wet?"wet":"dry");
		switch (prg_source) {
		case SRC_ROM:
			cmd_pointer = &washprogram[0];
			break;
		}
		wet_program = wet;
		cmd_state = STATE_FETCH_START ;
	}
}


unsigned char litterlanguage_running (void)
{
	return (cmd_state != STATE_IDLE);
}


void litterlanguage_pause (unsigned char pause)
{
	static struct {
		unsigned char	bowl;
		unsigned char	arm;
		unsigned char	pump;
		unsigned char	dosage;
		unsigned char	dryer;
		unsigned long	wait;
		unsigned long	fill;
		unsigned long	drain;
		unsigned long	autodose;
	} context;

	if (pause) {
		struct timer	timer_now;

		/* Save hardware context */
		context.bowl = get_Bowl();
		set_Bowl(BOWL_STOP);
		context.arm = get_Arm();
		set_Arm(ARM_STOP);
		context.dosage = get_Dosage();
		set_Dosage(0);
		context.pump = get_Pump();
		set_Pump(0);
		context.dryer = get_Dryer();
		set_Dryer(0);
		/* Save timer context */
		gettimestamp (&timer_now);
		/* We take a little shortcut here, not first checking timeoutexpired() and timeoutneverexpires()
		 * Expired timer will result in negative difference, returning 0: settimeout(0) during restoring will repoduced a practically expired timer
		 * Neverexpiring timer will result in huge difference, returning 0xFFFFFFFF: We will catch this during restoring */
		context.wait = timestampdiff(&timer_now, &timer_waitcmd);
		timeoutnever(&timer_waitcmd);
		context.fill = timestampdiff(&timer_now, &timer_fill);
		timeoutnever(&timer_fill);
		context.drain = timestampdiff(&timer_now, &timer_drain);
		timeoutnever(&timer_drain);
		context.autodose = timestampdiff(&timer_now, &timer_autodose);
		timeoutnever(&timer_autodose);
	} else {
		/* Restore timer context */
		if (context.wait != 0xFFFFFFFF)
			settimeout(&timer_waitcmd, context.wait);
		if (error_fill) {
			error_fill = 0;
			settimeout(&timer_fill, MAX_FILLTIME);
		}
		else if (context.fill != 0xFFFFFFFF)
			settimeout(&timer_fill, context.fill);
		if (error_drain) {
			error_drain = 0;
			settimeout(&timer_drain, MAX_DRAINTIME);
		} else if (context.drain != 0xFFFFFFFF)
			settimeout(&timer_drain, context.drain);
		if (context.autodose != 0xFFFFFFFF)
			settimeout(&timer_autodose, context.autodose);
		/* Restore hardware context */
		set_Bowl(context.bowl);
		set_Arm(context.arm);
		set_Dosage(context.dosage);
		set_Pump(context.pump);
		set_Dryer(context.dryer);
		error_dryer = 0;
	}
	paused = pause;
}


unsigned char litterlanguage_paused (void)
{
	return (paused);
}


void litterlanguage_stop (void)
{
	if (cmd_state != STATE_IDLE) {
		/* TODO: Stack overflow if we're printing here */
//		DBG("Stopping program\n");
		/* Stop all actuators */
		set_Bowl(BOWL_STOP);
		set_Arm(ARM_STOP);
		set_Water(0);
		set_Dosage(0);
		set_Pump(0);
		set_Dryer(0);
		/* Disable all timeouts */
		timeoutnever(&timer_fill);
		timeoutnever(&timer_drain);
		timeoutnever(&timer_autodose);
		/* Stop the state machine */
		cmd_state = STATE_IDLE;
		/* Reset pause sate */
		paused = 0;
	}
}


void watersensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	watersensor_event					      */
/*		- Handle state changes of water sensor			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	water_detected = detected;

	printtime();
	DBG("Water %s\n", water_detected?"high":"low");
	if (cmd_state != STATE_IDLE) {
		/* Disable timeout on event */
		if (water_detected) {
			/* Turn off the water and disable the timeout */
			set_Water(0);
			timeoutnever(&timer_fill);
			printtime();
			DBG("Filled\n");
		} else {
			printtime();
			DBG("Drained\n");
		}
	} else
		if (water_detected) {
			printtime();
			DBG("Box flooded!\n");
		}
}
/* watersensor_event */


void heatsensor_event (unsigned char detected)
/******************************************************************************/
/* Function:	heatsensor_event					      */
/*		- Handle state changes of over-heat sensor		      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	overheat_detected = detected;
}
/* heatsensor_event */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void litterlanguage_cleanup (unsigned char wet)
{
	extern const struct command	cleanupprogram[];
	if (cmd_state == STATE_IDLE) {
		prg_source = SRC_ROM;
		cmd_pointer = &cleanupprogram[0];
		wet_program = wet;
		cmd_state = STATE_FETCH_START ;
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
	unsigned int			temp;

//	DBG("IP 0x%04X: ", cmd_pointer);
	switch (cur_command.cmd) {
	case CMD_BOWL:
//		DBG("CMD_BOWL, %s", (cur_command.arg == BOWL_STOP)?"BOWL_STOP":((cur_command.arg == BOWL_CW)?"BOWL_CW":"BOWL_CCW"));
		set_Bowl((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_ARM:
//		DBG("CMD_ARM, %s", (cur_command.arg == ARM_STOP)?"ARM_STOP":((cur_command.arg == ARM_DOWN)?"ARM_DOWN":"ARM_UP"));
		set_Arm((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_WATER:
//		DBG("CMD_WATER, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			if (cur_command.arg) {
			      	if (eeprom_read(NVM_BOXSTATE) < BOX_WET)
					eeprom_write(NVM_BOXSTATE, BOX_WET);
				/* Don't fill if water is detected already */
				if (!water_detected) {
					printtime();
					DBG("Filling\n");
					set_Water(1);
					settimeout(&timer_fill, MAX_FILLTIME);
//					gettimestamp(&timer_fill);
				} //else
//					DBG(" (skipped)");
			} else {
				/* Disable timeout on filling */
				set_Water(0);
				timeoutnever(&timer_fill);
			}
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_PUMP:
//		DBG("CMD_PUMP, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program) {
			printtime();
			DBG("Draining\n");
			set_Pump((unsigned char)cur_command.arg);
		}
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_DRYER:
//		DBG("CMD_DRYER, %s%s", cur_command.arg?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Dryer((unsigned char)cur_command.arg);
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_WAITTIME:
//		DBG("CMD_WAITTIME, %ums", cur_command.arg);
		settimeout( &timer_waitcmd,
			    (unsigned long)cur_command.arg * MILISECOND );
		cmd_state = STATE_WAIT_CMD;
		break;
	case CMD_WAITWATER:
//		DBG("CMD_WAITWATER, %s%s", cur_command.arg?"high":"low", wet_program?"":" (nop)");
		if (wet_program) {
			if (!cur_command.arg)
				/* Start the drain timeout, we don't want to wait forever */
				settimeout(&timer_drain, MAX_DRAINTIME);
			cmd_state = STATE_WAIT_CMD;
		} else {
			cmd_pointer++;
			cmd_state = STATE_FETCH_CMD;
		}
		break;
	case CMD_WAITDOSAGE:
//		DBG("CMD_WAITDOSAGE%s", wet_program?"":" (nop)");
		if (wet_program)
			cmd_state = STATE_WAIT_CMD;
		else {
			cmd_pointer++;
			cmd_state = STATE_FETCH_CMD;
		}
		break;
	case CMD_SKIPIFDRY:
//		DBG("CMD_SKIPIFDRY, %u%s", cur_command.arg, wet_program?" (nop)":"");
		if (!wet_program)
			cmd_pointer += cur_command.arg + 1;
		else
			cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_SKIPIFWET:
//		DBG("CMD_SKIPIFWET, %u%s", cur_command.arg, wet_program?"":" (nop)");
		if (wet_program)
			cmd_pointer += cur_command.arg + 1;
		else
			cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_AUTODOSE:
//		DBG("CMD_AUTODOSE, %u.%uml%s", cur_command.arg/10, cur_command.arg%10, wet_program?"":" (nop)");
		if (wet_program) {
			settimeout(&timer_autodose,
				   (unsigned long)cur_command.arg * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
			set_Dosage(1);
		}
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_CALL:
//		DBG("CMD_CALL, 0x%04X", cur_command.arg);
		ret_address = cmd_pointer + 1;
		/* DIRTY HACK: Set highest bit, which seems to get lost due to compiler limitation */
		temp = 0x8000 | cur_command.arg;
		/* HACK: memcpy instead of casting to work around compiler limitation */
		memcpy(&cmd_pointer, &temp, sizeof(cmd_pointer));
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_RETURN:
//		DBG("CMD_RETURN, 0x%04X", ret_address);
		cmd_pointer = ret_address;
		cmd_state = STATE_FETCH_CMD;
		break;
	case CMD_END:
		printtime();
		DBG("CMD_END\n");
		eeprom_write(NVM_BOXSTATE, BOX_TIDY);
		litterlanguage_stop();
		break;
	case CMD_START:
//		DBG("CMD_START, unexpected");
		litterlanguage_stop();
		break;
	default:
		/* Program error */
//		DBG("CMD_unknown: 0x%X", cur_command.arg);
		litterlanguage_stop();
		break;
	}
//	DBG("\n");
}

static void wait_command (void)
{
	switch (cur_command.cmd) {
	case CMD_WAITTIME:
		if (timeoutexpired(&timer_waitcmd)) {
			cmd_pointer++;
			cmd_state = STATE_FETCH_CMD;
		}
		break;
	case CMD_WAITWATER:
		if (cur_command.arg) {
			if (water_detected) {
				cmd_pointer++;
				cmd_state = STATE_FETCH_CMD;
			}
		} else {
			if (!water_detected) {
				/* Disable the timeout */
				timeoutnever(&timer_drain);
				cmd_pointer++;
				cmd_state = STATE_FETCH_CMD;
			}
		}
		break;
	case CMD_WAITDOSAGE:
		if (!get_Dosage()) {
			cmd_pointer++;
			cmd_state = STATE_FETCH_CMD;
		}
		break;
	default:
		cmd_pointer++;
		cmd_state = STATE_FETCH_CMD;
		break;
	}
}
