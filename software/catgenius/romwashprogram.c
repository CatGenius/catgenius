/******************************************************************************/
/* File    :	romwashprogram.c					      */
/* Function:	CatGenius washing program				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	21 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "../common/hardware.h"		/* Flexible hardware configuration */

#include "romwashprogram.h"
#include "litterlanguage.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

//#define SIMULATION


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct instruction		const * ins_address = 0;

const struct instruction	drain[] = {
	{INS_PUMP,	1},		/* Wash + 15 */
	{INS_WAITTIME,	25206},
	{INS_PUMP,	0},		/* Wash + 16 */
	{INS_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{INS_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{INS_PUMP,	1},		/* Wash + 17 */
	{INS_WAITTIME,	24206},
	{INS_PUMP,	0},		/* Wash + 18 */
	{INS_WAITTIME,	8202},
	{INS_PUMP,	1},		/* Wash + 19 */
	{INS_WAITTIME,	24206},
	{INS_PUMP,	0},		/* Wash + 20 */
	{INS_WAITTIME,	8202},
	{INS_PUMP,	1},		/* Wash + 21 */
	{INS_WAITTIME,	65535},		/* Delay split in two because it exceeds maximum */
	{INS_WAITTIME,	10183},		/* 65535 + 10183 = 75718 */
	{INS_PUMP,	0},		/* Wash + 22 */
	{INS_WAITWATER, 0},
	{INS_RETURN,	0}
};

const struct instruction	drain_dry[] = {
	{INS_PUMP,	1},		/* Wash + 29 */
	{INS_BOWL,	BOWL_CCW},
	{INS_WAITTIME,	65535},
	{INS_BOWL,	BOWL_CW},	/* Wash + 30 */
	{INS_WAITTIME,	10075},
	{INS_BOWL,	BOWL_STOP},	/* Wash + 31 */
	{INS_AUTODOSE,	10},		/* 0.98 ml */
	{INS_WAITDOSAGE,0},
	{INS_BOWL,	BOWL_CW},	/* Wash + 32 */
	{INS_WAITTIME,	25270},
	{INS_BOWL,	BOWL_CCW},	/* Dry */
	{INS_DRYER,	1},
	{INS_WAITTIME,	35372},
	{INS_BOWL,	BOWL_CW},	/* Dry + 1 */
	{INS_WAITTIME,	55513},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 2 */
	{INS_WAITTIME,	35277},
	{INS_PUMP,	0},		/* Dry + 3 */
	{INS_WAITTIME,	1732},
	{INS_BOWL,	BOWL_CW},	/* Dry + 4 */
	{INS_WAITTIME,	55514},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 5 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CW},	/* Dry + 6 */
	{INS_WAITTIME,	55514},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 7 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CW},	/* Dry + 8 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 9 */
	{INS_WAITTIME,	35404},
	{INS_ARM,	ARM_UP},	/* Dry + 10 */
	{INS_WAITTIME,	9980},
	{INS_ARM,	ARM_STOP},	/* Dry + 11 */
	{INS_BOWL,	BOWL_STOP},
	{INS_AUTODOSE,	3},		/* 0.26 ml */
	{INS_WAITDOSAGE,0},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 12 */
	{INS_WAITTIME,	5392},
	{INS_ARM,	ARM_DOWN},	/* Dry + 13 */
	{INS_WAITTIME,	10303},
	{INS_ARM,	ARM_STOP},	/* Dry + 14 */
	{INS_WAITTIME,	35245},
	{INS_BOWL,	BOWL_CW},	/* Dry + 15 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 16 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CW},	/* Dry + 17 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 18 */
	{INS_WAITTIME,	35404},
	{INS_ARM,	ARM_UP},	/* Dry + 19 */
	{INS_WAITTIME,	10380},
	{INS_ARM,	ARM_STOP},	/* Dry + 20 */
	{INS_BOWL,	BOWL_STOP},
	{INS_AUTODOSE,	3},		/* 0.26 ml */
	{INS_WAITDOSAGE,0},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 21 */
	{INS_WAITTIME,	4392},
	{INS_ARM,	ARM_DOWN},	/* Dry + 22 */
	{INS_WAITTIME,	10803},
	{INS_ARM,	ARM_STOP},	/* Dry + 23 */
	{INS_WAITTIME,	35245},
	{INS_BOWL,	BOWL_CW},	/* Dry + 24 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 25 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CW},	/* Dry + 26 */
	{INS_WAITTIME,	35308},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 27 */
	{INS_WAITTIME,	35404},
	{INS_ARM,	ARM_UP},	/* Dry + 28 */
	{INS_WAITTIME,	11503},
	{INS_ARM,	ARM_STOP},	/* Dry + 29 */
	{INS_WAITTIME,	2169},
	{INS_ARM,	ARM_DOWN},	/* Dry + 30 */
	{INS_WAITTIME,	11703},
	{INS_ARM,	ARM_STOP},	/* Dry + 31 */
	{INS_WAITTIME,	45347},
	{INS_BOWL,	BOWL_CW},	/* Dry + 32 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 33 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CW},	/* Dry + 34 */
	{INS_WAITTIME,	35309},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 35 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CW},	/* Dry + 36 */
	{INS_WAITTIME,	35308},
	{INS_BOWL,	BOWL_CCW},	/* Dry + 37 */
	{INS_WAITTIME,	45411},
	{INS_BOWL,	BOWL_CW},	/* Dry + 38 */
	{INS_RETURN,	0}
};

const struct instruction	surface[] = {
	{INS_WAITTIME,	35404},
	{INS_ARM,	ARM_UP},	/* Dry + 39 */
	{INS_WAITTIME,	300},
	{INS_ARM,	ARM_STOP},	/* Dry + 40 */
	{INS_WAITTIME,	12203},
	{INS_ARM,	ARM_UP},	/* Dry + 41 */
	{INS_WAITTIME,	300},
	{INS_ARM,	ARM_STOP},	/* Dry + 42 */
	{INS_WAITTIME,	12203},
	{INS_ARM,	ARM_UP},	/* Dry + 43 */
	{INS_WAITTIME,	300},
	{INS_ARM,	ARM_STOP},	/* Dry + 44 */
	{INS_WAITTIME,	10203},
	{INS_ARM,	ARM_UP},	/* Dry + 45 */
	{INS_WAITTIME,	300},
	{INS_ARM,	ARM_STOP},	/* Dry + 46 */
	{INS_WAITTIME,	10202},
	{INS_ARM,	ARM_UP},	/* Dry + 47 */
	{INS_WAITTIME,	9170},
	{INS_ARM,	ARM_DOWN},	/* Dry + 48 */
	{INS_WAITTIME,	6474},
	{INS_DRYER,	0},		/* Dry + 49 */
	{INS_ARM,	ARM_UP},
	{INS_WAITTIME,	18268},
	{INS_ARM,	ARM_DOWN},	/* Dry + 50 */
	{INS_WAITTIME,	2832},
	{INS_ARM,	ARM_STOP},	/* Dry + 51 */
	{INS_RETURN,	0}
};

const struct instruction	cleanupprogram[] = {
	{INS_START,	INS_END |
			FLAGS_DRYRUN |
			FLAGS_WETRUN },
#ifdef SIMULATION
	{INS_WAITTIME,	3000},
#else /* SIMULATION */
	{INS_BOWL,	BOWL_CW},
	{INS_ARM,	ARM_DOWN},
	{INS_WAITTIME,	21769},
	{INS_ARM,	ARM_UP},
	{INS_WAITTIME,	932},
	{INS_ARM,	ARM_STOP},
	{INS_SKIPIFDRY, 1},		/* Skip to surfacing for dry program */
	/* Drain the bowl */
	{INS_CALL,	(unsigned int)drain_dry},
	/* Surface the granules */
	{INS_CALL,	(unsigned int)surface},
	{INS_BOWL,	BOWL_STOP},
#endif /* SIMULATION */
	{INS_END,	0}
};


const struct instruction	washprogram[] = {
	{INS_START,	INS_END |
			FLAGS_DRYRUN |
			FLAGS_WETRUN },
#ifdef SIMULATION
	{INS_WAITTIME,	3000},
#else /* SIMULATION */
	{INS_BOWL,	BOWL_CCW},	/* Scoop 1 */
	{INS_ARM,	ARM_DOWN},
	{INS_WAITTIME,	13217},
	{INS_ARM,	ARM_STOP},	/* Scoop 1 + 1 */
	{INS_WAITTIME,	18141},
	{INS_BOWL,	BOWL_CW},	/* Scoop 1 + 2 */
	{INS_WAITTIME,	6201},
	{INS_BOWL,	BOWL_CCW},	/* Scoop 1 + 3 */
	{INS_ARM,	ARM_DOWN},
	{INS_WAITTIME,	5765},
	{INS_ARM,	ARM_UP},	/* Scoop 1 + 4 */
	{INS_WAITTIME,	532},
	{INS_ARM,	ARM_STOP},	/* Scoop 1 + 5 */
	{INS_WAITTIME,	25206},
	{INS_ARM,	ARM_UP},	/* Scoop 1 + 6 */
	{INS_WAITTIME,	10671},
	{INS_ARM,	ARM_DOWN},	/* Scoop 1 + 7 */
	{INS_WAITTIME,	6602},
	{INS_ARM,	ARM_UP},	/* Scoop 1 + 8 */
	{INS_WAITTIME,	17204},
	{INS_ARM,	ARM_DOWN},	/* Scoop 2 */
	{INS_WAITTIME,	12703},
	{INS_ARM,	ARM_STOP},	/* Scoop 2 + 1 */
	{INS_WAITTIME,	4701},
	{INS_ARM,	ARM_DOWN},	/* Scoop 2 + 2 */
	{INS_WAITTIME,	11203},
	{INS_ARM,	ARM_UP},	/* Scoop 2 + 3 */
	{INS_WAITTIME,	532},
	{INS_ARM,	ARM_STOP},	/* Scoop 2 + 4 */
	{INS_WAITTIME,	25206},
	{INS_ARM,	ARM_UP},	/* Scoop 2 + 5 */
	{INS_WAITTIME,	10671},
	{INS_ARM,	ARM_DOWN},	/* Scoop 2 + 6 */
	{INS_WAITTIME,	6601},
	{INS_ARM,	ARM_UP},	/* Scoop 2 + 7 */
	{INS_WAITTIME,	20141},
	{INS_BOWL,	BOWL_CW},	/* Scoop 3 */
	{INS_ARM,	ARM_DOWN},
	{INS_WAITTIME,	21769},
	{INS_ARM,	ARM_UP},	/* Scoop 3 + 1 */
	{INS_WAITTIME,	932},
	{INS_ARM,	ARM_STOP},	/* Scoop 3 + 2 */

	{INS_SKIPIFDRY, 53},		/* Skip to surfacing for dry program */

	{INS_WAITTIME,	12108},
	{INS_BOWL,	BOWL_CCW},	/* Scoop 3 + 3 */
	{INS_ARM,	ARM_DOWN},
	{INS_WAITTIME,	3264},
	{INS_ARM,	ARM_UP},	/* Scoop 3 + 4 */
	{INS_WAITTIME,	532},
	{INS_ARM,	ARM_STOP},	/* Scoop 3 + 5 */
	{INS_WAITTIME,	24206},
	{INS_ARM,	ARM_UP},	/* Scoop 3 + 6 */
	{INS_WAITTIME,	10571},
	{INS_ARM,	ARM_DOWN},	/* Scoop 3 + 7 */
	{INS_WAITTIME,	6602},
	{INS_ARM,	ARM_UP},	/* Scoop 3 + 8 */
	{INS_WAITTIME,	17141},
	{INS_ARM,	ARM_STOP},	/* Scoop 3 + 9 */
	/* Wash the bowl */
	{INS_WAITWATER, 0},
	{INS_WATER,	1},
	{INS_BOWL,	BOWL_CW},
	{INS_WAITTIME,	18768},
	{INS_ARM,	ARM_DOWN},	/* Wash */
	{INS_WAITTIME,	25206},
	{INS_ARM,	ARM_UP},	/* Wash + 1 */
	{INS_WAITTIME,	1132},
	{INS_ARM,	ARM_STOP},	/* Wash + 2 */
	{INS_WAITWATER, 1},		/* Wash + 3 */
	{INS_WAITTIME,	63582},		/* From full program sheet */
	/* Drain the bowl */
	{INS_CALL,	(unsigned int)drain},
	/* Wash the bowl */
	{INS_WATER,	1},
	{INS_WAITTIME,	55418},
	{INS_BOWL,	BOWL_CCW},	/* Wash + 12 */
	{INS_AUTODOSE,	3},		/* 0.26 ml */
	{INS_WAITDOSAGE,0},
	{INS_BOWL,	BOWL_CW},
	{INS_WAITWATER, 1},		/* Wash + 14 */
	{INS_WAITTIME,	44002},		/* From full program sheet */
	/* Drain the bowl */
	{INS_CALL,	(unsigned int)drain},
	/* Wash the bowl */
	{INS_WATER,	1},
	{INS_WAITTIME,	25502},
	{INS_ARM,	ARM_DOWN},	/* Wash + 23 */
	{INS_WAITTIME,	21205},
	{INS_ARM,	ARM_UP},	/* Wash + 24 */
	{INS_BOWL,	BOWL_STOP},
	{INS_AUTODOSE,	11},		/* 1.07 ml */
	{INS_WAITTIME,	1132},
	{INS_ARM,	ARM_STOP},	/* Wash + 25 */
	{INS_WAITDOSAGE,0},
	{INS_BOWL,	BOWL_CCW},
	{INS_WAITTIME,	5329},
	{INS_BOWL,	BOWL_CW},	/* Wash + 27 */
	{INS_WAITTIME,	55482},
	{INS_WAITWATER, 1},		/* Wash + 28 */
	{INS_WAITTIME,	39107},
	/* Drain the bowl */
	{INS_CALL,	(unsigned int)drain_dry},
	/* Surface the granules */
	{INS_CALL,	(unsigned int)surface},
	{INS_BOWL,	BOWL_STOP},
#endif /* SIMULATION */
	{INS_END,	0}
};


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void romwashprogram_reqins (struct instruction const * address)
{
	ins_address = address;
}

unsigned char romwashprogram_getins (struct instruction * const instruction)
{
	*instruction = *ins_address;

	return 1;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/
