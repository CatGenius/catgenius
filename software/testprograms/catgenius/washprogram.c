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

#define CMD_BEGIN	0x01
#define CMD_BOWL	0x02
#define CMD_ARM		0x03
#define CMD_WATER	0x04
#define CMD_DOSAGE	0x05
#define CMD_PUMP	0x06
#define CMD_DRYER	0x07
#define CMD_WAITTIME	0x08
#define CMD_WAITWATER	0x09
#define CMD_SKIPIFDRY	0x10
#define CMD_SKIPIFWET	0x11
#define CMD_AUTODOSE	0x12
#define CMD_END		0xFE
#define CMD_RESERVED	0xFF

#define MAX_FILLTIME	((2*60+15)*SECOND)
#define MAX_DRAINTIME	((2*60+15)*SECOND) /* TODO: Tune this value */

/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

struct command {
	unsigned char	cmd;
	unsigned long	arg;
}

static const struct command	program[256] = {
	{CMD_BEGIN,     0},
	/* Scooping */
	{CMD_BOWL,      BOWL_CCW},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  13217 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  18141 * MILISECOND},
	{CMD_BOWL,      BOWL_CW},
	{CMD_WAITTIME,  6201 * MILISECOND},
	{CMD_BOWL,      BOWL_CCW},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  5765 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  532 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  10671 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  6602 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  17204 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  12703 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  4701 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  11203 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  532 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  10671 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  6601 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  20141 * MILISECOND},
	{CMD_BOWL,      BOWL_CW},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  21769 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  932 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  12108 * MILISECOND},
	{CMD_BOWL,      BOWL_CCW},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  3264 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  532 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  24206 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  10571 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  6602 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WATER,     1},
	{CMD_BOWL,      BOWL_CW},
	{CMD_WAITTIME,  17141 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_SKIPIFWET, 1},
	{CMD_BOWL,      BOWL_STOP},
	{CMD_SKIPIFDRY, 90},		/* TODO: Correct value to skip washing */
	/* Washing */
	{CMD_BOWL,      BOWL_CW},
	{CMD_WAITTIME,  18768 * MILISECOND},
	{CMD_ARM,       ARM_DOWN},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_ARM,       ARM_UP},
	{CMD_WAITTIME,  1132 * MILISECOND},
	{CMD_ARM,       ARM_STOP},
	{CMD_WAITTIME,  75591 * MILISECOND},
	{CMD_PUMP,      1},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_PUMP,      0},
	{CMD_WAITTIME,  75718 * MILISECOND},
	{CMD_PUMP,      1},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_PUMP,      0},
	{CMD_WAITTIME,  8202 * MILISECOND},
	{CMD_PUMP,      1},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_PUMP,      0},
	{CMD_WAITTIME,  8202 * MILISECOND},
	{CMD_PUMP,      1},
	{CMD_WAITTIME,  65616 * MILISECOND},
	{CMD_PUMP,      0},
	{CMD_WATER,     1},
	{CMD_BOWL,      BOWL_CCW},
	{CMD_DOSAGE,    1},
	{CMD_WAITTIME,  55418 * MILISECOND},
	{CMD_WAITTIME,  2601 * MILISECOND},
	{CMD_WAITTIME,  65711 * MILISECOND},
	{CMD_WAITTIME,  25206 * MILISECOND},
	{CMD_WAITTIME,  75718 * MILISECOND},
	{CMD_WAITTIME,  24206 * MILISECOND},
	{CMD_WAITTIME,  8202 * MILISECOND},
	{CMD_WAITTIME,  24206 * MILISECOND},
	{CMD_WAITTIME,  8202 * MILISECOND},
	{CMD_WAITTIME,  75718 * MILISECOND},
	{CMD_WAITTIME,  25502 * MILISECOND},
	{CMD_WAITTIME,  21205 * MILISECOND},
	{CMD_WAITTIME,  1132 * MILISECOND},
	{CMD_WAITTIME,  9580 * MILISECOND},
	{CMD_WAITTIME,  5329 * MILISECOND},
	{CMD_WAITTIME,  55482 * MILISECOND},
	{CMD_WAITTIME,  65648 * MILISECOND},
	{CMD_END,       0}
};
#if 0
			settimeout(&state_timer, 25174*MILISECOND);
			settimeout(&state_timer, 1132*MILISECOND);
			settimeout(&state_timer, 1132*MILISECOND);
	{CMD_WATER,     0);
			settimeout(&state_timer, 63582*MILISECOND);
			settimeout(&state_timer, 25602*MILISECOND);
			settimeout(&state_timer, 75718*MILISECOND);
			settimeout(&state_timer, 25602*MILISECOND);
			settimeout(&state_timer, 8202*MILISECOND);
			settimeout(&state_timer, 25602*MILISECOND);
			settimeout(&state_timer, 8202*MILISECOND);
			settimeout(&state_timer, 65616*MILISECOND);
			settimeout(&state_timer, 2462*MILISECOND);
	{CMD_DOSAGE,    0);
	{CMD_BOWL,       BOWL_CW);
			settimeout(&state_timer, 0*MILISECOND);
		if (detected_Water()) {
	{CMD_WATER,     0);
			settimeout(&state_timer, 63582*MILISECOND);
			state++;
		} else
			if (timeoutexpired(&water_timer)) {
		{CMD_WATER,     0);
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
	/* Check timeouts */
	if (timeoutexpired(&timer_autodose)) {
		set_Dosage(0);
		timeoutnever(&timer_autodose);
	}
	if (timeoutexpired(&timer_fill)) {
		/* Fill error */
		/* Pauze */
	}
	if (timeoutexpired(&timer_drain)) {
		/* Drain error */
		/* Pauze */
	}

	/* Do the waiting */
	switch (program[pc].cmd) {
	case CMD_WAITTIME:
		if (!timeoutexpired(&timer_waitcmd))
			return;
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
		pc++;
		break;
	}

	/* Execute wait commands */
	switch (program[pc].cmd) {
	case CMD_BEGIN:
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
		settimeout(&timer_waitcmd, program[pc].arg);
		break;
	case CMD_WAITWATER:
		break;
	case CMD_SKIPIFDRY:
		if (scooponly)
			pc += program[pc].arg + 1;
		break;
	case CMD_SKIPIFWET:
		if (!scooponly)
			pc += program[pc].arg + 1;
		break;
	case CMD_AUTODOSE:
		settimeout(&timer_autodose, program[pc].arg);
		set_Dosage(1);
		pc++;
		break;
	case CMD_END:
		timeoutnever(&timer_drain);
		pc=0;
		break;
	default:
		/* Program error */
		timeoutnever(&timer_autostart);
		pc = 0;
		break;
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
	if (!pc)
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

