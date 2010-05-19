/******************************************************************************/
/* File    :	romwashprogram.c					      */
/* Function:	CatGenius washing program				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "romwashprogram.h"
#include "litterlanguage.h"
#include "../common/catgenie120.h"
#include "../common/hardware.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

//#define SIMULATION


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct command		const * cmd_address = 0;

#if 0
static const struct command	cleanupprogram[] = {
	{CMD_START,	CMD_LAST | 
			FLAGS_DRYRUN |
			FLAGS_WETRUN },
#ifdef SIMULATION
	{CMD_WAITTIME,	3000},
#else /* SIMULATION */
	{CMD_BOWL,	BOWL_CW},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	ARM_STROKE_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	500},
	{CMD_ARM,	ARM_STOP},
	{CMD_SKIPIFDRY, },

	/* Pumping */
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	BOWL_REV_MSEC},
	{CMD_PUMP,	0},
	{CMD_WAITTIME,	BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_PUMP,	0},
	{CMD_WAITTIME,	BOWL_REV_MSEC / 2},
	{CMD_BOWL,	BOWL_CW},
	{CMD_PUMP,	1},
	{CMD_WAITTIME,	3 * BOWL_REV_MSEC},
	{CMD_PUMP,	0},

	/* Drying */
	{CMD_DRYER,	1},
	{CMD_WAITTIME,	4 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	3 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	4 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	4 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	3 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	3 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITTIME,	3 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	2 * BOWL_REV_MSEC},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 24 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 25 */
	{CMD_WAITTIME,	35309},
	{CMD_BOWL,	BOWL_CW},	/* Dry + 26 */
	{CMD_WAITTIME,	35308},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 27 */
	{CMD_WAITTIME,	35404},
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

	/* Surfacing */
	{CMD_WAITTIME,	BOWL_REV_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 45},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	BOWL_REV_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 45},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	BOWL_REV_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 45},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	BOWL_REV_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 45},
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	BOWL_REV_MSEC + 1000},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	3 * (ARM_STROKE_MSEC / 4) + 1000},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 2},
	{CMD_DRYER,	0},
	{CMD_BOWL,	BOWL_STOP},
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	3 * (ARM_STROKE_MSEC / 4)},
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	ARM_STROKE_MSEC / 5},
	{CMD_ARM,	ARM_STOP},
#endif /* SIMULATION */
	{CMD_END,	0}
};
#endif

const struct command	washprogram[] = {
	{CMD_START,	CMD_LAST | 
			FLAGS_DRYRUN |
			FLAGS_WETRUN },
#ifdef SIMULATION
	{CMD_WAITTIME,	3000},
#else /* SIMULATION */
	{CMD_BOWL,	BOWL_CCW},	/* Scoop 1 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	13217},
	{CMD_ARM,	ARM_STOP},	/* Scoop 1 + 1 */
	{CMD_WAITTIME,	18141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop 1 + 2 */
	{CMD_WAITTIME,	6201},
	{CMD_BOWL,	BOWL_CCW},	/* Scoop 1 + 3 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	5765},
	{CMD_ARM,	ARM_UP},	/* Scoop 1 + 4 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop 1 + 5 */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Scoop 1 + 6 */
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},	/* Scoop 1 + 7 */
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},	/* Scoop 1 + 8 */
	{CMD_WAITTIME,	17204},
	{CMD_ARM,	ARM_DOWN},	/* Scoop 2 */
	{CMD_WAITTIME,	12703},
	{CMD_ARM,	ARM_STOP},	/* Scoop 2 + 1 */
	{CMD_WAITTIME,	4701},
	{CMD_ARM,	ARM_DOWN},	/* Scoop 2 + 2 */
	{CMD_WAITTIME,	11203},
	{CMD_ARM,	ARM_UP},	/* Scoop 2 + 3 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop 2 + 4 */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Scoop 2 + 5 */
	{CMD_WAITTIME,	10671},
	{CMD_ARM,	ARM_DOWN},	/* Scoop 2 + 6 */
	{CMD_WAITTIME,	6601},
	{CMD_ARM,	ARM_UP},	/* Scoop 2 + 7 */
	{CMD_WAITTIME,	20141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop 3 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	21769},
	{CMD_ARM,	ARM_UP},	/* Scoop 3 + 1 */
	{CMD_WAITTIME,	932},
	{CMD_ARM,	ARM_STOP},	/* Scoop 3 + 2 */

	{CMD_SKIPIFDRY, 179},		/* Skip to surfacing in dry program */

	{CMD_WAITTIME,	12108},
	{CMD_BOWL,	BOWL_CCW},	/* Scoop 3 + 3 */
	{CMD_ARM,	ARM_DOWN},
	{CMD_WAITTIME,	3264},
	{CMD_ARM,	ARM_UP},	/* Scoop 3 + 4 */
	{CMD_WAITTIME,	532},
	{CMD_ARM,	ARM_STOP},	/* Scoop 3 + 5 */
	{CMD_WAITTIME,	24206},
	{CMD_ARM,	ARM_UP},	/* Scoop 3 + 6 */
	{CMD_WAITTIME,	10571},
	{CMD_ARM,	ARM_DOWN},	/* Scoop 3 + 7 */
	{CMD_WAITTIME,	6602},
	{CMD_ARM,	ARM_UP},	/* Scoop 3 + 8 */
	{CMD_WAITWATER, 0},
	{CMD_WATER,	1},
	{CMD_WAITTIME,	17141},
	{CMD_BOWL,	BOWL_CW},	/* Scoop 3 + 9 */
	{CMD_ARM,	ARM_STOP},
	{CMD_WAITTIME,	18768},
	{CMD_ARM,	ARM_DOWN},	/* Wash */
	{CMD_WAITTIME,	25206},
	{CMD_ARM,	ARM_UP},	/* Wash + 1 */
	{CMD_WAITTIME,	1132},
	{CMD_ARM,	ARM_STOP},	/* Wash + 2 */
	{CMD_WAITWATER, 1},		/* Wash + 3 */
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
	{CMD_WAITWATER, 0},
	{CMD_WATER,	1},
	{CMD_WAITTIME,	55418},	
	{CMD_BOWL,	BOWL_CCW},	/* Wash + 12 */
	{CMD_AUTODOSE,	3},		/* 0.26 ml */
//	{CMD_DOSAGE,	1},
	{CMD_WAITDOSAGE,0},
//	{CMD_WAITTIME,	2601},
//	{CMD_DOSAGE,	0},		/* Wash + 13 */
	{CMD_BOWL,	BOWL_CW},
	{CMD_WAITWATER, 1},		/* Wash + 14 */
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
	{CMD_WAITWATER, 0},
	{CMD_WATER,	1},
	{CMD_WAITTIME,	25502},
	{CMD_ARM,	ARM_DOWN},	/* Wash + 23 */
	{CMD_WAITTIME,	21205},
	{CMD_ARM,	ARM_UP},	/* Wash + 24 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_AUTODOSE,	11},		/* 1.07 ml */
//	{CMD_DOSAGE,	1},
//	{CMD_WAITTIME,	1 * (DOSAGE_SECONDS_PER_ML / 10) * 1000},
	{CMD_WAITTIME,	1132},
	{CMD_ARM,	ARM_STOP},	/* Wash + 25 */
	{CMD_WAITDOSAGE,0},
//	{CMD_WAITTIME,	9580},
//	{CMD_DOSAGE,	0},		/* Wash + 26 */
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	5329},
	{CMD_BOWL,	BOWL_CW},	/* Wash + 27 */
	{CMD_WAITTIME,	55482},
	{CMD_WAITWATER, 1},		/* Wash + 28 */
	{CMD_WAITTIME,	39107},
	{CMD_PUMP,	1},		/* Wash + 29 */
	{CMD_BOWL,	BOWL_CCW},
	{CMD_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{CMD_WAITTIME,	113},		/* 65535 + 113 = 65648 */
	{CMD_BOWL,	BOWL_CW},	/* Wash + 30 */
	{CMD_WAITTIME,	10075},
	{CMD_BOWL,	BOWL_STOP},	/* Wash + 31 */
	{CMD_AUTODOSE,	10},		/* 0.98 ml */
//	{CMD_DOSAGE,	1},
	{CMD_WAITDOSAGE,0},
//	{CMD_WAITTIME,	9802},
	{CMD_BOWL,	BOWL_CW},	/* Wash + 32 */
//	{CMD_DOSAGE,	0},
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
	{CMD_AUTODOSE,	3},		/* 0.26 ml */
//	{CMD_DOSAGE,	1},
	{CMD_WAITDOSAGE,0},
//	{CMD_WAITTIME,	2601},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 12 */
//	{CMD_DOSAGE,	0},
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
	{CMD_AUTODOSE,	3},		/* 0.26 ml */
//	{CMD_DOSAGE,	1},
	{CMD_WAITDOSAGE,0},
//	{CMD_WAITTIME,	2601},
	{CMD_BOWL,	BOWL_CCW},	/* Dry + 21 */
//	{CMD_DOSAGE,	0},
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
	/* Surfacing */
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
	{CMD_DRYER,	0},		/* Dry + 49 */
	{CMD_ARM,	ARM_UP},
	{CMD_WAITTIME,	18268},
	{CMD_ARM,	ARM_DOWN},	/* Dry + 50 */
	{CMD_BOWL,	BOWL_STOP},
	{CMD_WAITTIME,	2832},
	{CMD_ARM,	ARM_STOP},	/* Dry + 51 */
#endif /* SIMULATION */
	{CMD_END,	0}
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void romwashprogram_reqcmd (struct command const * address)
{
	cmd_address = address;
}

unsigned char romwashprogram_getcmd (struct command * const command)
{
	*command = *cmd_address;

	return 1;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
