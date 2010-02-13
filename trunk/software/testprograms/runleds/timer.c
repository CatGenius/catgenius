/******************************************************************************/
/* File    :	timer.h							      */
/* Function:	Timer module						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>

#include "timer.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static unsigned long		overflows	= 0;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void timer_init (void)
/******************************************************************************/
/* Function:	Timer module initialisation routine			      */
/*		- Initializes the module				      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
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
/* Function:	Timer module initialisation routine			      */
/*		- Terminates the module					      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	TMR1IE = 0;		/* Disable timer interrupt */
	TMR1ON = 0;		/* Stop timer1 */
}
/* End: timer_term */


void settimeout (struct timer	* const timer_p,
		 unsigned long	  const timout)
/******************************************************************************/
/* Function:	settimeout						      */
/*		- Calculate a requested timeout				      */
/* History :	11 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	unsigned long	tempLongTicks;		/* Overflow detection buffer */
	struct {
		unsigned long	ls_longTicks ;	/* Least significant long */
		unsigned short	ms_shortTicks ;	/* Most significant short */
	} *longshort = timer_p;

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


#if 0
void TMR_PostponeTimeout (struct timer * const timer_p,
                          unsigned long      const postpone)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_PosponeTimeout                                         //
//                 - Add the requested timeout to the given timeout           //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned long       tempTicksWord;  // Rollover detection buffer

  // Temporarilly store the Long-part of the given timeout
  tempTicksWord = timer_p->t_ticksCalc.u32_lsLong;

  // Add the timout to the Long-part of the given timeout
  timer_p->t_ticksCalc.u32_lsLong += postpone;

  // Check if the Long-part has rolled over and increase
  // the Short-part if so
  if (timer_p->t_ticksCalc.u32_lsLong < tempTicksWord)
  {
    timer_p->t_ticksCalc.u16_msShort ++;
  }

  return;
}
// End: TMR_PosponeTimeout
#endif


void timeoutnow (struct timer * const timer_p)
/******************************************************************************/
/* Function:	timeoutnow						      */
/*		- Expire the given timer immediately			      */
/* History :	13 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
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
/*		- Initial revision.					      */
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
/*		- Initial revision.					      */
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


void gettimestamp (struct timer * const ticks)
/******************************************************************************/
/* Function:	gettimestamp:						      */
/*		- Fetch current ticks accurately, without stopping the timer  */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
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


#if 0
unsigned long TMR_TimeStampAge (struct timer const * const pt_timestamp)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_TimeStampAge                                           //
//                 - Calculate the time now and the time the give timestamp   //
//                   was set                                                  //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned long       age         = 0;
  struct timer    currentTime;

  // Fetch the current time
  gettimestamp (&currentTime);

  // Check Most Significant Short for future time stamps
  if (currentTime.t_ticksCalc.u16_msShort >= pt_timestamp->t_ticksCalc.u16_msShort)
  {
    switch (currentTime.t_ticksCalc.u16_msShort - pt_timestamp->t_ticksCalc.u16_msShort)
    {
      case 0: // Most Significant Short is the same
        if (currentTime.t_ticksCalc.u32_lsLong > pt_timestamp->t_ticksCalc.u32_lsLong)
        {
          // Past time stamp: Just subtract the Least Significant LWords
          age = currentTime.t_ticksCalc.u32_lsLong - pt_timestamp->t_ticksCalc.u32_lsLong;
        }
        else
        {
          // Future or current time stamp
          age = 0;
        }
        break;

      case 1: // Most Significant LWord is increased just by 1
        if (currentTime.t_ticksCalc.u32_lsLong < pt_timestamp->t_ticksCalc.u32_lsLong)
        {
          // The Least Significant LWords has only wrapped around
          age = (0xFFFFFFFF - (pt_timestamp->t_ticksCalc.u32_lsLong - currentTime.t_ticksCalc.u32_lsLong)) + 1;
        }
        else
        {
          // The The stamp has just aged beyond the maximum
          age = 0xFFFFFFFF;
        }
        break;

      default: // Most Significant LWord is increased by two of more
        // The time stamp is ancient, older than 49 days!
        age = 0xFFFFFFFF;
        break;
    }
  }
  else
  {
    // Future time stamp
    age = 0;
  }

  return (age);
}
// End: TMR_TimeStampAge


void TMR_Delay (unsigned long const delay)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_Delay                                                  //
//                 - Hold up the processor for the specified nr of ticks      //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  struct timer timeoutTime;

  // Set a timout
  TMR_SetTimeout (&timeoutTime, delay);

  // Wait for it to expire
  while (!TMR_CheckTimeout (&timeoutTime))
  {
    // Just wait...
;
  }

  return;
}
// End: TMR_Delay


void TMR_MicroDelay (unsigned short const delay_us)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_Delay                                                  //
//                 - Hold up the processor for the specified nr of ticks      //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
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
#endif


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

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
}
/* End: timer1_isr */
