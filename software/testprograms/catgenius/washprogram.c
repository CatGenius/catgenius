/******************************************************************************/
/* File    :	washprogram.c						      */
/* Function:	CatGenius washing program				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "washprogram.h"
#include "../common/timer.h"
#include "../common/catgenie120.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define FLAGS_AUTORUN	0x0100	/* Start program without user intervention */
#define FLAGS_DRYRUN	0x0200	/* Program supports dry cleaning */
#define FLAGS_WETRUN	0x0400	/* Program supports wet cleaning */
#define FLASH_OVERWRITE	0x0800	/* Program will be copied to FLASH to replace standard program */

#define CMD_START	0x00	/* Designates the start of a program. Argument is CMD_LAST or-ed with flags */
#define CMD_BOWL	0x01	/* Controls the bowl. Argument is what the bowl should do */
#define CMD_ARM		0x02	/* Controls the arm. Argument is what the arm should do */
#define CMD_WATER	0x03	/* Controls the water valve. Argument is */
#define CMD_DOSAGE	0x04
#define CMD_PUMP	0x05
#define CMD_DRYER	0x06

#define CMD_WAITTIME	0x08
#define CMD_WAITWATER	0x09
#define CMD_SKIPIFDRY	0x10
#define CMD_SKIPIFWET	0x11
#define CMD_AUTODOSE	0x12
#define CMD_LAST	CMD_AUTODOSE
#define CMD_END		0xFE
#define CMD_RESERVED	0xFF

#define MAX_FILLTIME	((2*60+15)*SECOND)
#define MAX_DRAINTIME	((2*60+15)*SECOND) /* TODO: Tune this value */

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

struct command {
	unsigned char	cmd;
	unsigned int	arg;
}

static const struct command	program[] = {
	{CMD_START,	CMD_LAST | 
			FLAGS_DRYRUN |
			FLAGS_WETRUN },
	/* Scooping */
	{CMD_BOWL,	BOWL_CCW},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	13217},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	18141},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	6201},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	5765},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	17204},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	12703},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	4701},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	11203},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	6601},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	20141},
	{CMD_BOWL,	BOWL_CW},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	21769},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	932},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	12108},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	3264},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	24206},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	10571},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},
	{CMD_WATER,	1},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	17141},
	{CMD_ARM,	ARM_STOP},
	{CMD_SKIPIFWET, 1},
	{CMD_BOWL,	BOWL_STOP},
{CMD_END, 0},
	{CMD_SKIPIFDRY, 90},		/* TODO: Correct value to skip washing */
	/* Washing */
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	18768},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	1132},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	10056},		/* 65535 + 10056 = 75591 */
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	65616},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	81},		/* 65535 + 81 = 65616 */
	{CMD_PUMP,	0},
	{CMD_WATER,	1},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	55418},
	{CMD_WAITTIME,	2601},
	{CMD_WAITTIME,	65711},
	{CMD_WAITTIME,	25206},
	{CMD_WAITTIME,	75718},
	{CMD_WAITTIME,	24206},
	{CMD_WAITTIME,	8202},
	{CMD_WAITTIME,	24206},
	{CMD_WAITTIME,	8202},
	{CMD_WAITTIME,	75718},
	{CMD_WAITTIME,	25502},
	{CMD_WAITTIME,	21205},
	{CMD_WAITTIME,	1132},
	{CMD_WAITTIME,	9580},
	{CMD_WAITTIME,	5329},
	{CMD_WAITTIME,	55482},
	{CMD_WAITTIME,	65648},
	{CMD_END,	0}
};
#if 0
			settimeout(&state_timer, 25174);
			settimeout(&state_timer, 1132);
			settimeout(&state_timer, 1132);
	{CMD_WATER,	0);
			settimeout(&state_timer, 63582);
			settimeout(&state_timer, 25602);
			settimeout(&state_timer, 75718);
			settimeout(&state_timer, 25602);
			settimeout(&state_timer, 8202);
			settimeout(&state_timer, 25602);
			settimeout(&state_timer, 8202);
			settimeout(&state_timer, 65616);
			settimeout(&state_timer, 2462);
	{CMD_DOSAGE,	0);
	{CMD_BOWL,	BOWL_CW);
			settimeout(&state_timer, 0);
		if (detected_Water()) {
	{CMD_WATER,	0);
			settimeout(&state_timer, 63582);
			state++;
		} else
			if (timeoutexpired(&water_timer)) {
		{CMD_WATER,	0);
				/*
				 * ERROR 2
				 */
			}
		break;
#endif

static struct timer	timer_autostart = {0xFFFF, 0xFFFFFFFF};
static struct timer	timer_waitcmd   = {0xFFFF, 0xFFFFFFFF};
static struct timer	timer_fill      = {0xFFFF, 0xFFFFFFFF};
static struct timer	timer_drain     = {0xFFFF, 0xFFFFFFFF};
static struct timer	timer_autodose  = {0xFFFF, 0xFFFFFFFF};
static unsigned char	scooponly   = 1;
static unsigned char	pc       = 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

static void wait ();
static void execute ();


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void washprogram_init (void)
/******************************************************************************/
/* Function:	washprogram_init					      */
/*		- Initialize the CatGenius washing program		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* washprogram_init */

#define MAX_FILL_TIME	(135*SECOND)
#define SCOOP		0
#define WASH		27
#define DRY		100
#define EQUALIZE	120

void washprogram_work (void)
/******************************************************************************/
/* Function:	washprogram_work					      */
/*		- Worker function for the CatGenius washing program	      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	static bit	waiting = 0;

	/* Check error timeouts */
	if (timeoutexpired(&timer_fill)) {
		/* Fill error */
		/* Pauze */
	}
	if (timeoutexpired(&timer_drain)) {
		/* Drain error */
		/* Pauze */
	}

	/* Check auto timeouts */
	if (timeoutexpired(&timer_autodose)) {
		set_Dosage(0);
		timeoutnever(&timer_autodose);
	}

	/* Do the waiting */
	if (waiting) {
		switch (program[pc].cmd) {
		case CMD_WAITTIME:
			if (!timeoutexpired(&timer_waitcmd))
				return;
			waiting = 0;
			pc++;
			break;
		case CMD_WAITWATER:
			if (program[pc].arg) {
				if (!detected_Water())
					return;
				timeoutnever(&timer_fill);
			} else {
				if (detected_Water())
					return;
				timeoutnever(&timer_drain);
			}
			waiting = 0;
			pc++;
			break;
		}
	} else {
	/* Execute wait commands */
	switch (program[pc].cmd) {
	case CMD_START:
		if (timeoutexpired(&timer_autostart))
			pc++;
		break;
	case CMD_BOWL:
		set_Bowl((unsigned char)program[pc].arg);
		pc++;
		break;
	case CMD_ARM:
		set_Arm((unsigned char)program[pc].arg);
		pc++;
		break;
	case CMD_WATER:
		if (!scooponly) {
			set_Water((unsigned char)program[pc].arg);
			if (program[pc].arg)
				settimeout(&timer_fill, MAX_FILLTIME);
			else
				timeoutnever(&timer_fill);
		}
		pc++;
		break;
	case CMD_DOSAGE:
		set_Dosage((unsigned char)program[pc].arg);
		pc++;
		break;
	case CMD_PUMP:
		set_Pump((unsigned char)program[pc].arg);
		if (program[pc].arg)
			settimeout(&timer_drain, MAX_DRAINTIME);
		else
			timeoutnever(&timer_drain);
		pc++;
		break;
	case CMD_DRYER:
		set_Dryer((unsigned char)program[pc].arg);
		pc++;
		break;
	case CMD_WAITTIME:
		settimeout( &timer_waitcmd,
			    (unsigned long)program[pc].arg * MILISECOND );
		waiting = 1;
		break;
	case CMD_WAITWATER:
		waiting = 1;
		break;
	case CMD_SKIPIFDRY:
		if (scooponly)
			pc += program[pc].arg + 1;
		else
			pc++;
		break;
	case CMD_SKIPIFWET:
		if (!scooponly)
			pc += program[pc].arg + 1;
		else
			pc++;
		break;
	case CMD_AUTODOSE:
		settimeout(&timer_autodose, program[pc].arg);
		set_Dosage(1);
		pc++;
		break;
	case CMD_END:
		timeoutnever(&timer_drain);
		timeoutnever(&timer_autostart);
		pc=0;
		break;
	default:
		/* Program error */
		timeoutnever(&timer_autostart);
		pc = 0;
		break;
	}
}
}
/* washprogram_work */


void washprogram_term (void)
/******************************************************************************/
/* Function:	userinterface_term					      */
/*		- Terminate the CatGenius washing program		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
}
/* washprogram_term */


void washprogram_mode (unsigned char justscoop)
{
	if (!pc)
		scooponly = justscoop ;
}


void washprogram_start (void)
{
	if (program[pc].cmd == CMD_START)
		timeoutnow(&timer_autostart);
}


unsigned char washprogram_running (void)
{
	return (pc);
}


void washprogram_pause (unsigned char pause)
{
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void wait ()
{
}

static void execute ()
{
}
