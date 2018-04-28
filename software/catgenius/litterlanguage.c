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
#include "../common/water.h"
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
#define STATE_FETCH_INS		3
#define STATE_GET_INS		4
#define STATE_WAIT_INS		5

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned char		prg_source		= 0;
static bit			wet_program		= 0;
static bit			overheat_detected	= 0;
static bit			paused			= 0;
static bit			error_fill		= 0;
static bit			error_drain		= 0;
static bit			error_flood		= 0;
static bit			error_dryer		= 0;

/* Program execution variables */
static unsigned char		ins_state		= STATE_IDLE;
static struct instruction	const * ins_pointer	= 0;
static struct instruction	cur_instruction;

static struct timer		timer_waitins		= NEVER;
static struct timer		timer_fill		= NEVER;
static struct timer		timer_drain		= NEVER;
static struct timer		timer_autodose		= NEVER;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void		litterlanguage_cleanup (unsigned char wet);
static void		req_instruction (struct instruction	const *instruction);
static unsigned char	get_instruction (struct instruction	*instruction);
static void		exe_instruction (void);
static void		wait_instruction (void);


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
				DBG("tidy\n");
				break;
			case BOX_MESSY:
				DBG("messy\n");
				litterlanguage_cleanup(0);
				break;
			case BOX_WET:
				DBG("wet\n");
				litterlanguage_cleanup(1);
				break;
			default:
				DBG("unknown\n");
				eeprom_write(NVM_BOXSTATE, BOX_TIDY);
				break;
			}
			break;
		case START_BUTTON:
			/* User wants to force a wet cleanup cycle */
			DBG("Wet cleanup forced\n");
			litterlanguage_cleanup(1);
			break;
		case SETUP_BUTTON:
			/* User wants to use GenieDiag */
			break;
		case START_BUTTON | SETUP_BUTTON:
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
	if( (ins_state != STATE_IDLE) &&
	    !paused ){
		/* Check for filling timeout */
		if( !water_detected() &&
		    water_filling() &&
		    timeoutexpired(&timer_fill) ){
			printtime();
			DBG("Fill timeout\n");
			timeoutnever(&timer_fill);
			/* Fill error */
			error_fill = 1;
			/* Pauze */
//			litterlanguage_pause(1);
			litterlanguage_stop();
			set_LED_Error(0x01, 1);
			set_Beeper(0x01, 1);
		}

		/* Check for draining timeout */
		if( water_detected() &&
		    get_Pump() &&
		    timeoutexpired(&timer_drain) ){
			printtime();
			DBG("Drain timeout\n");
			timeoutnever(&timer_drain);
			/* Drain error */
			error_drain = 1;
			/* Pauze */
//			litterlanguage_pause(1);
			litterlanguage_stop();
			set_LED_Error(0x05, 1);
			set_Beeper(0x05, 1);
		}
		/* Check for overheating error */
		if( overheat_detected &&
		    get_Dryer() ){
			printtime();
			DBG("Overheating\n");
			/* Overheat error */
			error_dryer = 1;
			/* Pauze */
//			litterlanguage_pause(1);
			litterlanguage_stop();
			set_LED_Error(0x15, 1);
			set_Beeper(0x015, 1);
		}
	}

	/* Check auto instruction timeouts */
	if (timeoutexpired(&timer_autodose)) {
		timeoutnever(&timer_autodose);
		set_Dosage(0);
	}

	switch (ins_state) {
	case STATE_IDLE:	/* Idle */
		break;

	case STATE_FETCH_START:	/* Fetch the start instruction */
		req_instruction(ins_pointer);
		ins_state = STATE_GET_START;
		/* no break; */

	case STATE_GET_START:	/* Wait for the start instruction to be fetched */
		if (get_instruction(&cur_instruction)) {
//			DBG("IP 0x%04X: ", ins_pointer);
			if (cur_instruction.opcode == INS_START) {
//				DBG("INS_START, %s", wet_program?"wet":"dry");
				/* Check if this is a valid program for us */
				if( ((cur_instruction.operant & 0x00FF) <= INS_END) &&
				    ( (!wet_program && (cur_instruction.operant & FLAGS_DRYRUN)) ||
				      (wet_program && (cur_instruction.operant & FLAGS_WETRUN)) ) ) {
					if (eeprom_read(NVM_BOXSTATE) < BOX_MESSY)
						eeprom_write(NVM_BOXSTATE, BOX_MESSY);
					ins_pointer++;
					ins_state = STATE_FETCH_INS;
				} else {
					ins_state = STATE_IDLE;
//					DBG(", incompatible");
				}
			} else {
				ins_state = STATE_IDLE;
//				DBG(", no start: 0x%X", cur_instruction.opcode);
			}
//			DBG("\n");
		}
		break;

	case STATE_FETCH_INS:	/* Fetch the next instruction */
		req_instruction(ins_pointer);
		ins_state = STATE_GET_INS;
		/* no break; */

	case STATE_GET_INS:	/* Wait for the start instruction to be fetched */
		if (get_instruction(&cur_instruction)) {
			/* Decode and execute the instruction */
			exe_instruction();
		}
		break;

	case STATE_WAIT_INS:	/* Wait for the instruction to finish */
		wait_instruction();
		break;
	}
}
/* litterlanguage_work */


void litterlanguage_start (unsigned char wet)
{
	extern const struct instruction	washprogram[];
	if (ins_state == STATE_IDLE) {
		printtime();
		DBG("Starting %s program\n", wet?"wet":"dry");
		switch (prg_source) {
		case SRC_ROM:
			ins_pointer = &washprogram[0];
			break;
		}
		wet_program = wet;
		ins_state = STATE_FETCH_START ;
	}
}


unsigned char litterlanguage_running (void)
{
	return (ins_state != STATE_IDLE);
}


void litterlanguage_pause (unsigned char pause)
{
	static struct {
		unsigned	bowl	: 2;
		unsigned	arm	: 2;
		unsigned	water	: 1;
		unsigned	pump	: 1;
		unsigned	dosage	: 1;
		unsigned	dryer	: 1;
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
		context.water = water_filling();
		water_fill(0);
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
		context.wait = timestampdiff(&timer_now, &timer_waitins);
		timeoutnever(&timer_waitins);
		context.fill = timestampdiff(&timer_now, &timer_fill);
		timeoutnever(&timer_fill);
		context.drain = timestampdiff(&timer_now, &timer_drain);
		timeoutnever(&timer_drain);
		context.autodose = timestampdiff(&timer_now, &timer_autodose);
		timeoutnever(&timer_autodose);
	} else {
		/* Restore timer context */
		if (context.wait != 0xFFFFFFFF)
			settimeout(&timer_waitins, context.wait);
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
		water_fill(context.water);
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
	if (ins_state != STATE_IDLE) {
		printtime();
		DBG("Stopping program\n");
		/* Stop all actuators */
		set_Bowl(BOWL_STOP);
		set_Arm(ARM_STOP);
		water_fill(0);
		set_Dosage(0);
		set_Pump(0);
		set_Dryer(0);
		/* Disable all timeouts */
		timeoutnever(&timer_fill);
		timeoutnever(&timer_drain);
		timeoutnever(&timer_autodose);
		/* Stop the state machine */
		ins_state = STATE_IDLE;
		/* Reset pause state */
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
	printtime();
	DBG("Water %s\n", detected?"high":"low");
	if (ins_state != STATE_IDLE) {
		/* Disable timeout on event */
		if (detected) {
			/* Turn off the water and disable the timeout */
			water_fill(0);
			timeoutnever(&timer_fill);
			printtime();
			DBG("Filled\n");
		} else {
			printtime();
			DBG("Drained\n");
		}
	} else
		if (detected) {
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
	extern const struct instruction	cleanupprogram[];
	if (ins_state == STATE_IDLE) {
		printtime();
		DBG("Starting %s cleanup\n", wet?"wet":"dry");
		prg_source = SRC_ROM;
		ins_pointer = &cleanupprogram[0];
		wet_program = wet;
		ins_state = STATE_FETCH_START ;
	}
}


static void req_instruction (struct instruction const *instruction)
{
	switch (prg_source) {
	case SRC_ROM:
		romwashprogram_reqins(instruction);
	}
}

static unsigned char get_instruction (struct instruction *instruction)
{
	switch (prg_source) {
	default:
	case SRC_ROM:
		return romwashprogram_getins(instruction);
	}
}

static void exe_instruction (void)
{
	static struct instruction	const * ret_address;
	unsigned int			temp;

//	DBG("IP 0x%04X: ", ins_pointer);
	switch (cur_instruction.opcode) {
	case INS_BOWL:
//		DBG("INS_BOWL, %s", (cur_instruction.operant == BOWL_STOP)?"BOWL_STOP":((cur_instruction.operant == BOWL_CW)?"BOWL_CW":"BOWL_CCW"));
		set_Bowl((unsigned char)cur_instruction.operant);
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_ARM:
//		DBG("INS_ARM, %s", (cur_instruction.operant == ARM_STOP)?"ARM_STOP":((cur_instruction.operant == ARM_DOWN)?"ARM_DOWN":"ARM_UP"));
		set_Arm((unsigned char)cur_instruction.operant);
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_WATER:
//		DBG("INS_WATER, %s%s", cur_instruction.operant?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			if (cur_instruction.operant) {
				if (eeprom_read(NVM_BOXSTATE) < BOX_WET)
					eeprom_write(NVM_BOXSTATE, BOX_WET);
				/* Don't fill if water is detected already */
				if (!water_detected()) {
					printtime();
					DBG("Filling\n");
					water_fill(1);
					settimeout(&timer_fill, MAX_FILLTIME);
//					gettimestamp(&timer_fill);
				} //else
//					DBG(" (skipped)");
			} else {
				/* Disable timeout on filling */
				water_fill(0);
				timeoutnever(&timer_fill);
			}
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_PUMP:
//		DBG("INS_PUMP, %s%s", cur_instruction.operant?"on":"off", wet_program?"":" (nop)");
		if (wet_program) {
			printtime();
			DBG("Draining\n");
			set_Pump((unsigned char)cur_instruction.operant);
		}
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_DRYER:
//		DBG("INS_DRYER, %s%s", cur_instruction.operant?"on":"off", wet_program?"":" (nop)");
		if (wet_program)
			set_Dryer((unsigned char)cur_instruction.operant);
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_WAITTIME:
//		DBG("INS_WAITTIME, %ums", cur_instruction.operant);
		settimeout( &timer_waitins,
			    (unsigned long)cur_instruction.operant * MILISECOND );
		ins_state = STATE_WAIT_INS;
		break;
	case INS_WAITWATER:
//		DBG("INS_WAITWATER, %s%s", cur_instruction.operant?"high":"low", wet_program?"":" (nop)");
		if (wet_program) {
			if (!cur_instruction.operant)
				/* Start the drain timeout, we don't want to wait forever */
				settimeout(&timer_drain, MAX_DRAINTIME);
			ins_state = STATE_WAIT_INS;
		} else {
			ins_pointer++;
			ins_state = STATE_FETCH_INS;
		}
		break;
	case INS_WAITDOSAGE:
//		DBG("INS_WAITDOSAGE%s", wet_program?"":" (nop)");
		if (wet_program)
			ins_state = STATE_WAIT_INS;
		else {
			ins_pointer++;
			ins_state = STATE_FETCH_INS;
		}
		break;
	case INS_SKIPIFDRY:
//		DBG("INS_SKIPIFDRY, %u%s", cur_instruction.operant, wet_program?" (nop)":"");
		if (!wet_program)
			ins_pointer += cur_instruction.operant + 1;
		else
			ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_SKIPIFWET:
//		DBG("INS_SKIPIFWET, %u%s", cur_instruction.operant, wet_program?"":" (nop)");
		if (wet_program)
			ins_pointer += cur_instruction.operant + 1;
		else
			ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_AUTODOSE:
//		DBG("INS_AUTODOSE, %u.%uml%s", cur_instruction.operant/10, cur_instruction.operant%10, wet_program?"":" (nop)");
		if (wet_program) {
			settimeout(&timer_autodose,
				   (unsigned long)cur_instruction.operant * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
			set_Dosage(1);
		}
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_CALL:
//		DBG("INS_CALL, 0x%04X", cur_instruction.operant);
		ret_address = ins_pointer + 1;
		/* DIRTY HACK: Set highest bit, which seems to get lost due to compiler limitation */
		temp = 0x8000 | cur_instruction.operant;
		/* HACK: memcpy instead of casting to work around compiler limitation */
		memcpy(&ins_pointer, &temp, sizeof(ins_pointer));
		ins_state = STATE_FETCH_INS;
		break;
	case INS_RETURN:
//		DBG("INS_RETURN, 0x%04X", ret_address);
		ins_pointer = ret_address;
		ins_state = STATE_FETCH_INS;
		break;
	case INS_END:
//		DBG("INS_END\n");
		eeprom_write(NVM_BOXSTATE, BOX_TIDY);
		litterlanguage_stop();
		break;
	case INS_START:
//		DBG("INS_START, unexpected");
		litterlanguage_stop();
		break;
	default:
		/* Program error */
//		DBG("INS_unknown: 0x%X", cur_instruction.operant);
		litterlanguage_stop();
		break;
	}
//	DBG("\n");
}

static void wait_instruction (void)
{
	switch (cur_instruction.opcode) {
	case INS_WAITTIME:
		if (timeoutexpired(&timer_waitins)) {
			ins_pointer++;
			ins_state = STATE_FETCH_INS;
		}
		break;
	case INS_WAITWATER:
		if (cur_instruction.operant) {
			if (water_detected()) {
				ins_pointer++;
				ins_state = STATE_FETCH_INS;
			}
		} else {
			if (!water_detected()) {
				/* Disable the timeout */
				timeoutnever(&timer_drain);
				ins_pointer++;
				ins_state = STATE_FETCH_INS;
			}
		}
		break;
	case INS_WAITDOSAGE:
		if (!get_Dosage()) {
			ins_pointer++;
			ins_state = STATE_FETCH_INS;
		}
		break;
	default:
		ins_pointer++;
		ins_state = STATE_FETCH_INS;
		break;
	}
}
