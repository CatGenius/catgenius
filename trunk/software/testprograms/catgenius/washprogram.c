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
	{CMD_BOWL,	BOWL_CCW},	/* Scoop */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	13217},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 1 */
	{CMD_WAITTIME,	18141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop + 2 */
	{CMD_WAITTIME,	6201},
	{CMD_BOWL,	BOWL_CCW},	/* Scoop + 3 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	5765},
	{CMD_ARM,	ARM_UP},	/* Scoop + 4 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 5 */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Scoop + 6 */
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},	/* Scoop + 7 */
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},	/* Scoop + 8 */
	{CMD_WAITTIME,	17204},
	{CMD_ARM,	ARM_DOWN},	/* Scoop + 9 */
	{CMD_WAITTIME,	12703},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 10 */
	{CMD_WAITTIME,	4701},
	{CMD_ARM,	ARM_DOWN},	/* Scoop + 11 */
	{CMD_WAITTIME,	11203},
	{CMD_ARM,	ARM_UP},	/* Scoop + 12 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 13 */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Scoop + 14 */
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},	/* Scoop + 15 */
	{CMD_WAITTIME,	6601},
	{CMD_ARM,	ARM_UP},	/* Scoop + 16 */
	{CMD_WAITTIME,	20141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop + 17 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	21769},
	{CMD_ARM,	ARM_UP},	/* Scoop + 18 */
	{CMD_WAITTIME,	932},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 19 */
	{CMD_WAITTIME,	12108},
	{CMD_BOWL,	BOWL_CCW},	/* Scoop + 20 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	3264},
	{CMD_ARM,	ARM_UP},	/* Scoop + 21 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop + 22 */
	{CMD_WAITTIME,	24206},
	{CMD_ARM,	ARM_UP},	/* Scoop + 23 */
	{CMD_WAITTIME,	10571},
	{CMD_ARM,	ARM_DOWN},	/* Scoop + 24 */
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},	/* Scoop + 25 */
	{CMD_WATER,	1},
	{CMD_WAITTIME,	17141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop + 26 */
	{CMD_ARM,	ARM_STOP},
//	{CMD_SKIPIFWET, 2},
//	{CMD_BOWL,	BOWL_STOP},
//	{CMD_SKIPIFDRY, 90},		/* TODO: Correct value to skip washing */
	{CMD_WAITTIME,	18768},
	{CMD_ARM,	ARM_DOWN},	/* Wash */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Wash + 1 */
	{CMD_WAITTIME,	1132},
	{CMD_ARM,	ARM_STOP},	/* Wash + 2 */
	{CMD_WAITWATER, 1},		/* Wash + 3 */
	{CMD_WATER,	0},
	{CMD_WAITTIME,	63582},		/* From full program sheet */
	{CMD_PUMP,	1},		/* Wash + 4 */
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},		/* Wash + 5 */
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{CMD_PUMP,	1},		/* Wash + 6 */
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},		/* Wash + 7 */
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},		/* Wash + 8 */
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},		/* Wash + 9 */
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},		/* Wash + 10 */
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	81},		/* 65535 + 81 = 65616 */
	{CMD_PUMP,	0},		/* Wash + 11 */
	{CMD_WATER,	1},
	{CMD_WAITTIME,	55418},	
	{CMD_BOWL,	BOWL_CCW},	/* Wash + 12 */
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	2601},
	{CMD_DOSAGE,	0},		/* Wash + 13 */
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITWATER, 1},		/* Wash + 14 */
	{CMD_WATER,	0},
	{CMD_WAITTIME,	44002},		/* From full program sheet */
	{CMD_PUMP,	1},		/* Wash + 15 */
	{CMD_WAITTIME,	25206},
	{CMD_PUMP,	0},		/* Wash + 16 */
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{CMD_PUMP,	1},		/* Wash + 17 */
	{CMD_WAITTIME,	24206},
	{CMD_PUMP,	0},		/* Wash + 18 */
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},		/* Wash + 19 */
	{CMD_WAITTIME,	24206},
	{CMD_PUMP,	0},		/* Wash + 20 */
	{CMD_WAITTIME,	8202},
	{CMD_PUMP,	1},		/* Wash + 21 */
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{CMD_PUMP,	0},		/* Wash + 22 */
	{CMD_WATER,	1},
	{CMD_WAITTIME,	25502},
	{CMD_ARM,	ARM_DOWN},	/* Wash + 23 */
	{CMD_WAITTIME,	21205},
	{CMD_ARM,	ARM_UP},	/* Wash + 24 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	1132},
	{CMD_ARM,	ARM_STOP},	/* Wash + 25 */
	{CMD_WAITTIME,	9580},
	{CMD_DOSAGE,	0},		/* Wash + 26 */
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	5329},
	{CMD_BOWL,	BOWL_CW},	/* Wash + 27 */
	{CMD_WAITTIME,	55482},
	{CMD_WAITWATER, 1},		/* Wash + 28 */
	{CMD_WATER,	0},
	{CMD_WAITTIME,	39107},
	{CMD_PUMP,	1},		/* Wash + 29 */
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	113},		/* 65535 + 113 = 65648 */
	{CMD_BOWL,	BOWL_CW},	/* Wash + 30 */
	{CMD_WAITTIME,	10075},
	{CMD_BOWL,	BOWL_STOP},	/* Wash + 31 */
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	9802},
	{CMD_BOWL,	BOWL_CW},	/* Wash + 32 */
	{CMD_DOSAGE,	0},
	{CMD_WAITTIME,	25270},
	{CMD_BOWL,	BOWL_CCW},	/* Dry */
	{CMD_DRYER,	1},
	{CMD_WAITTIME,	35372},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 1 */
	{CMD_WAITTIME,	55513},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 2 */
	{CMD_WAITTIME,	35277},
	{CMD_PUMP,	0},		/* Dry + 3 */
	{CMD_WAITTIME,	1732},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 4 */
	{CMD_WAITTIME,	55514},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 5 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 6 */
	{CMD_WAITTIME,	55514},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 7 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 8 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 9 */
	{CMD_WAITTIME,	35404},
	{CMD_ARM,	ARM_UP},	/* Dry + 10 */
	{CMD_WAITTIME,	9980},
	{CMD_ARM,	ARM_STOP},	/* Dry + 11 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	2601},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 12 */
	{CMD_DOSAGE,	0},
	{CMD_WAITTIME,	5392},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 13 */
	{CMD_WAITTIME,	10303},
	{CMD_ARM,	ARM_STOP},	/* Dry + 14 */
	{CMD_WAITTIME,	35245},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 15 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 16 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 17 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 18 */
	{CMD_WAITTIME,	35404},
	{CMD_ARM,	ARM_UP},	/* Dry + 19 */
	{CMD_WAITTIME,	10380},
	{CMD_ARM,	ARM_STOP},	/* Dry + 20 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_DOSAGE,	1},
	{CMD_WAITTIME,	2601},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 21 */
	{CMD_DOSAGE,	0},
	{CMD_WAITTIME,	4392},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 22 */
	{CMD_WAITTIME,	10803},
	{CMD_ARM,	ARM_STOP},	/* Dry + 23 */
	{CMD_WAITTIME,	35245},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 24 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 25 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 26 */
	{CMD_WAITTIME,	35308},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 27 */
	{CMD_WAITTIME,	35404},
	{CMD_ARM,	ARM_UP},	/* Dry + 28 */
	{CMD_WAITTIME,	11503},
	{CMD_ARM,	ARM_STOP},	/* Dry + 29 */
	{CMD_WAITTIME,	2169},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 30 */
	{CMD_WAITTIME,	11703},
	{CMD_ARM,	ARM_STOP},	/* Dry + 31 */
	{CMD_WAITTIME,	45347},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 32 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 33 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 34 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 35 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 36 */
	{CMD_WAITTIME,	35308},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 37 */
	{CMD_WAITTIME,	45411},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 38 */
	{CMD_WAITTIME,	35404},
	{CMD_ARM,	ARM_UP},	/* Dry + 39 */
	{CMD_WAITTIME,	300},
	{CMD_ARM,	ARM_STOP},	/* Dry + 40 */
	{CMD_WAITTIME,	12203},
	{CMD_ARM,	ARM_UP},	/* Dry + 41 */
	{CMD_WAITTIME,	300},
	{CMD_ARM,	ARM_STOP},	/* Dry + 42 */
	{CMD_WAITTIME,	12203},
	{CMD_ARM,	ARM_UP},	/* Dry + 43 */
	{CMD_WAITTIME,	300},
	{CMD_ARM,	ARM_STOP},	/* Dry + 44 */
	{CMD_WAITTIME,	10203},
	{CMD_ARM,	ARM_UP},	/* Dry + 45 */
	{CMD_WAITTIME,	300},
	{CMD_ARM,	ARM_STOP},	/* Dry + 46 */
	{CMD_WAITTIME,	10202},
	{CMD_ARM,	ARM_UP},	/* Dry + 47 */
	{CMD_WAITTIME,	9170},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 48 */
	{CMD_WAITTIME,	6474},
	{CMD_ARM,	ARM_UP},	/* Dry + 49 */
	{CMD_WAITTIME,	18268},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 50 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_WAITTIME,	2832},
	{CMD_ARM,	ARM_STOP},	/* Dry + 51 */
	{CMD_END,	0}
};


static struct timer	timer_autostart = NEVER;
static struct timer	timer_waitcmd   = NEVER;
static struct timer	timer_fill      = NEVER;
static struct timer	timer_drain     = NEVER;
static struct timer	timer_autodose  = NEVER;
static unsigned char	scooponly       = 0;
static unsigned char	pc              = 0;


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
