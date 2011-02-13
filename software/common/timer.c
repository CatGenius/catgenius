/******************************************************************************/
/* File    :	timer.c							      */
/* Function:	Timer module						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 1999-2010, Clockwork Engineering		      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "timer.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

struct longshort {
	unsigned long	ls_longTicks ;	/* Least significant long */
	unsigned short	ms_shortTicks ;	/* Most significant short */
} ;


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static volatile unsigned long	overflows	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void timer_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	/*
	 * Initialize timer 1
	 */
	/* Select Fosc/4 as source */
	TMR1CS = 0;
	/* Set prescaler to 1:8 */
	T1CKPS1 = 1;
	T1CKPS0 = 1;
	/* Disable timer 1 external oscillator */
	T1OSCEN = 0;
	/* Disable synchronized clock */
	T1SYNC = 0;
	/* Switch on timer 1 */
	TMR1ON = 1;
	/* Enable timer 1 interrupt */
	TMR1IE = 1;
}
/* End: timer_init */


void timer_term (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Terminates the module					      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	/* Disable timer interrupt */
	TMR1IE = 0;
	/* Stop timer1 */
	TMR1ON = 0;
}
/* End: timer_term */


void settimeout (struct timer	* const timer_p,
		 unsigned long	  const timout)
/******************************************************************************/
/* Function:	settimeout						      */
/*		- Calculate a requested timeout				      */
/* History :	11 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	unsigned long		tempLongTicks;		/* Overflow detection buffer */
	struct longshort	*longshort = (struct longshort*)timer_p;

	/* Fetch the current time */
	gettimestamp(timer_p);

	/* If no timeout is required, we're done */
	if (!timout)
		return;

	/* Store lower value-before-adding to detect an overflow */
	tempLongTicks = longshort->ls_longTicks;

	/* Add the requested delay */
	longshort->ls_longTicks += timout;

	/* Carry an overflow to the most significant part */
	if (longshort->ls_longTicks <= tempLongTicks)
		longshort->ms_shortTicks++;
}
/* End: settimeout */


void postponetimeout (struct timer	* const timer_p,
		      unsigned long	  const postpone)
/******************************************************************************/
/* Function:	postponetimeout						      */
/*		- Add the requested timeout to the given timer		      */
/* History :	25 Aug 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	unsigned long		tempLongTicks;		/* Overflow detection buffer */
	struct longshort	*longshort = (struct longshort*)timer_p;

	/* If no postponement is required, we're done */
	if (!postpone)
		return;

	/* Store lower value-before-adding to detect an overflow */
	tempLongTicks = longshort->ls_longTicks;

	/* Add the requested postponement */
	longshort->ls_longTicks += postpone;

  	/* Carry an overflow to the most significant part */
	if (longshort->ls_longTicks <= tempLongTicks)
		longshort->ms_shortTicks++;
}
/* End: postponetimeout */


void timeoutnow (struct timer * const timer_p)
/******************************************************************************/
/* Function:	timeoutnow						      */
/*		- Expire the given timer immediately			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	timer_p->timer1    = 0x0000;
	timer_p->overflows = 0x00000000;
}
/* End: timeoutnow */


void timeoutnever (struct timer * const timer_p)
/******************************************************************************/
/* Function:	timeoutnever						      */
/*		- Make sure the given timer will not expire anymore	      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	timer_p->timer1    = 0xFFFF;
	timer_p->overflows = 0xFFFFFFFF;
}
/* End: timeoutnever */


unsigned char timeoutexpired (struct timer const * const timer_p)
/******************************************************************************/
/* Function:	timeoutexpired						      */
/*		- Check if a given timeout has expired			      */
/* History :	11 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	struct timer	currentTime;

	/* Fetch the current time */
	gettimestamp (&currentTime);

	if (currentTime.overflows == timer_p->overflows)
		if (currentTime.timer1 >= timer_p->timer1)
			return 1;
		else
			return 0;
	else
		if (currentTime.overflows > timer_p->overflows)
			return 1;
		else
			return 0;
}
/* End: timeoutexpired */


unsigned char timeoutneverexpires (struct timer	 const	* const timer_p)
/******************************************************************************/
/* Function:	timeoutneverexpires					      */
/*		- Check if a given timeout is disabled			      */
/* History :	11 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	return (timer_p->timer1    == 0xFFFF &&
		timer_p->overflows == 0xFFFFFFFF);
}


void gettimestamp (struct timer * const ticks)
/******************************************************************************/
/* Function:	gettimestamp:						      */
/*		- Fetch current ticks accurately, without stopping the timer  */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	volatile unsigned char	temp;
	unsigned char		*bytes = (unsigned char *)ticks;

	/* To prevent a roll-over while reading the separate bytes, we could
	 * disable the interrupts, but this would reduce the ticks accuracy.
	 * That's why we use a trick: */
	do
	{
		/* Temporarilly store the first roll-over dependent byte
		 * directly from the running clock */
		temp = TMR1H;

		/* Copy the contents from the running clock into the return buffer */
		ticks->overflows = overflows;
		bytes[0] = TMR1L;
		bytes[1] = TMR1H;

		/* Check if the Least Significant Byte has rolled over during this
		 * operation, having the incread a More Significant Byte and making
		 * the buffer invalid */
	} while (bytes[1] != temp);
}
/* End: gettimestamp */


unsigned long timestampage (struct timer const * const pt_timestamp)
/******************************************************************************/
/* Function:	timestampage						      */
/*		- Calculate the time between now and the time the given	      */
/*		  timestamp was set					      */
/* History :	25 Aug 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	unsigned long		age = 0;
	struct longshort	cur_longshort;
	struct longshort	*tmr_longshort = (struct longshort*)pt_timestamp;

	/* Fetch the current time */
	gettimestamp ((struct timer*)&cur_longshort);

	/* Check overflows to detect a future time stamp */
	if (cur_longshort.ms_shortTicks >= tmr_longshort->ms_shortTicks) {
		switch (cur_longshort.ms_shortTicks - tmr_longshort->ms_shortTicks) {
		case 0: /* Most Significant Shorts are equal */
			if (cur_longshort.ls_longTicks > tmr_longshort->ls_longTicks)
				/* Past time stamp: Just subtract the Least Significant Longs */
				age = cur_longshort.ls_longTicks - tmr_longshort->ls_longTicks;
			break;

		case 1: /* Most Significant Long is increased just by 1 */
			if (cur_longshort.ls_longTicks < tmr_longshort->ls_longTicks)
				/* The Least Significant Long has only wrapped around */
				age = (0xFFFFFFFF - (tmr_longshort->ls_longTicks - cur_longshort.ls_longTicks)) + 1;
			else
				/* The The stamp has just aged beyond the maximum of 9.5 hours */
				age = 0xFFFFFFFF;
			break;

		default: /* Most Significant Long is increased by two of more */
			/* The time stamp is ancient, way older than 9.5 hours */
			age = 0xFFFFFFFF;
			break;
		}
	}

	return (age);
}
/* End: timestampage */


#if 0
void delay (unsigned long const delay)
/******************************************************************************/
/* Function:	delay							      */
/*		- Hold up the processor for the specified nr of ticks	      */
/* History :	25 Aug 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	struct timer timeoutTime;

	/* Set a timout */
	TMR_SetTimeout (&timeoutTime, delay);

	/* Wait for it to expire */
	while (!TMR_CheckTimeout (&timeoutTime))
	{
		/* Just wait... */
		;
	}

	return;
}
/* End: delay */


void microdelay (unsigned short const delay_us)
/******************************************************************************/
/* Function:	microdelay						      */
/*		- Hold up the processor for the specified nr microseconds     */
/* History :	25 Aug 2010 by R. Delien:				      */
/*		- Ported from other project.				      */
/******************************************************************************/
{
	unsigned char            u8_old_tmr3_dr[2];
	unsigned short volatile *pu16_old_tmr3 = (unsigned short volatile *)&(u8_old_tmr3_dr[0]);

	unsigned char            u8_cur_tmr3_dr[2];
	unsigned short volatile *pu16_cur_tmr3 = (unsigned short volatile *)&(u8_cur_tmr3_dr[0]);

	unsigned int             u24_ticks_to_wait;
	unsigned int             u24_ticks_waited = 0;

	// Store TMR3 value first, for highest accuracy
	u8_old_tmr3_dr[0] = TMR3_DR_L; // Read the lower byte first, this will also latch the higher byte
	u8_old_tmr3_dr[1] = TMR3_DR_H; // Read the latched higher byte

	// Ticks-per-us is 12.5. To keep the 0.5, we use ticks-per-10us which is 125.
	// To calculate the proper nr of ticks to wait, the delay must be multiplied
	// by 10 too.
	u24_ticks_to_wait = (delay_us * TICKS_PER_10_US) / 10;

	while (u24_ticks_waited < u24_ticks_to_wait)
	{
		// Fetch the current TMR3 value
		u8_cur_tmr3_dr[0] = TMR3_DR_L; // Read the lower byte first, this will also latch the higher byte
		u8_cur_tmr3_dr[1] = TMR3_DR_H; // Read the latched higher byte

		// Check if the timer has decreased and if it rolled under
		if (*pu16_cur_tmr3 < *pu16_old_tmr3)
		{
			// Timer has decreased, just add the difference
			u24_ticks_waited += (*pu16_old_tmr3 - *pu16_cur_tmr3);
		}
		else if (*pu16_cur_tmr3 > *pu16_old_tmr3)
		{
			// Timer has rolled under.
			u24_ticks_waited += *pu16_old_tmr3 + (TMR3_RELOAD_VALUE - *pu16_cur_tmr3);
		}

		*pu16_old_tmr3 = *pu16_cur_tmr3;
	}

	return;
}
/* End: microdelay */
#endif


void timer_isr (void)
/******************************************************************************/
/* Function:	Timer 1 interrupt service routine:			      */
/*		- Each interrupt will increment the 32-bit ticks counter.     */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	/* Increase the number of timer overflows */
	overflows++;

	/* If the combined timer ever reaches 0xFFFF00000000, the maximum settable
	 * timeout of 0xFFFFFFFF will be equal to the magic value for NEVER. This
	 * would seriously mess things up, so all we can do is reset, after a
	 * respectable up-time of 71 years */
	if (overflows >= 0xFFFF0000) {
		overflows = 0;
		/* TODO: This print make the linker crash... */
//		printf("RIP\n");
		while(1);
	}
}
/* End: timer1_isr */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

