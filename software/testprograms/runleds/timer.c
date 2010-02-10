/******************************************************************************/
/* File    :	timer.h							      */
/* Function:	Timer module						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#define TIMER_C

#include <htc.h>

#include "timer.h"

#define NR_OF_TICK_BYTES  (6)

#if 0
#define Bit(n)            (1U << (n))           // Bit mask for bit 'n'
#define MaskShiftL(i,m,s) (((i) & (m)) << (s))  // Mask 'i' with 'm' and shift left 's'
#define MaskShiftR(i,m,s) (((i) & (m)) >> (s))  // Mask 'i' with 'm' and shift right 's'

#define B0_MASK           0x01
#define B1_MASK           0x02
#define B2_MASK           0x04
#define B3_MASK           0x08
#define B4_MASK           0x10
#define B5_MASK           0x20
#define B6_MASK           0x40
#define B7_MASK           0x80


#define TICKS_PER_10_US       (125)
#define TMR3_RELOAD_VALUE     (0x30D4)
#define T3_max_value          0xFFFF
#define nsecs_per_T3_inc      400
#define nsecs_per_tick        (TICKS_PER_MS * 1000000UL)
#define T3_incs_per_tick      (nsecs_per_tick / nsecs_per_T3_inc)
#define T3_reload_value       ((T3_max_value - T3_incs_per_tick) + 1)
#endif

////////////////////////////////////////////////////////////////////////////////
// Global Data                                                                //
////////////////////////////////////////////////////////////////////////////////

static unsigned char		test = 0;
static unsigned char		ticks[NR_OF_TICK_BYTES - 2] ;

////////////////////////////////////////////////////////////////////////////////
// Local Prototypes                                                           //
////////////////////////////////////////////////////////////////////////////////

//static void ISR_Timer3       (void) ;
//static void TMR_CurrentTicks (TMR_ticks_struct * const currTicks_ptr) ;


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
	TMR1CS = 0 ;
	/* Set prescaler to 1:8 */
	T1CKPS1 = 1 ;
	T1CKPS0 = 1 ;
	/* Disable timer 1 external oscillator */
	T1OSCEN = 0 ;
	/* Disable synchronized clock */
	T1SYNC = 0;
	/* Switch on timer 1 */
	TMR1ON = 1 ;
	/* Enable timer 1 interrupt */
	TMR1IE = 1 ;

	return;
}
// End: timer_init


void timer_term (void)
/******************************************************************************/
/* Function:	Timer module initialisation routine			      */
/*		- Terminates the module					      */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	TMR1IE = 0 ;		/* Disable timer interrupt */
	TMR1ON = 0 ;		/* Stop timer1 */

	return;
}
// End: timer_term

#if 0
void TMR_SetTimeout (TMR_ticks_struct * const pt_timeout,
                     unsigned long      const timout)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_SetTimeout                                             //
//                 - Calculate a requested timeout                            //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned long       tempTicksWord ;  // Rollover detection buffer

  // Fetch the current time
  TMR_CurrentTicks (pt_timeout) ;

  // Temporarilly store the Long-part of the current time
  tempTicksWord = pt_timeout->t_ticksCalc.u32_lsLong ;

  // Add the timout to the Long-part of the current time
  pt_timeout->t_ticksCalc.u32_lsLong += timout ;

  // Check if the Long-part has rolled over and increase
  // the Short-part if so
  if (pt_timeout->t_ticksCalc.u32_lsLong < tempTicksWord)
  {
    pt_timeout->t_ticksCalc.u16_msShort ++ ;
  }

  return ;
}
// End: TMR_SetTimeout


void TMR_PostponeTimeout (TMR_ticks_struct * const pt_timeout,
                          unsigned long      const postpone)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_PosponeTimeout                                         //
//                 - Add the requested timeout to the given timeout           //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned long       tempTicksWord ;  // Rollover detection buffer

  // Temporarilly store the Long-part of the given timeout
  tempTicksWord = pt_timeout->t_ticksCalc.u32_lsLong ;

  // Add the timout to the Long-part of the given timeout
  pt_timeout->t_ticksCalc.u32_lsLong += postpone ;

  // Check if the Long-part has rolled over and increase
  // the Short-part if so
  if (pt_timeout->t_ticksCalc.u32_lsLong < tempTicksWord)
  {
    pt_timeout->t_ticksCalc.u16_msShort ++ ;
  }

  return ;
}
// End: TMR_PosponeTimeout


void TMR_ExpireTimeout (TMR_ticks_struct * const pt_timeout)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_ExpireTimeout                                          //
//                 - Make the given timeout expire unconditionally            //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned char cntr ;

  // Set timeout time to 0 by clearing all bytes
  for (cntr = 0; cntr < NR_OF_TICK_BYTES; cntr ++)
  {
    pt_timeout->u8_ticksByte[cntr] = 0x00 ;
  }

  return ;
}
// End: TMR_ExpireTimeout


void TMR_NeverTimeout (TMR_ticks_struct * const pt_timeout)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_NeverTimeout                                           //
//                 - Prevent the given timeout to expire, ever                //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned char cntr ;

  // Set timeout time to 'nearly' infinite (>584942417 year)
  for (cntr = 0; cntr < NR_OF_TICK_BYTES; cntr ++)
  {
    pt_timeout->u8_ticksByte[cntr] = 0xFF ;
  }

  return ;
}
// End: TMR_NeverTimeout


BOOL TMR_CheckTimeout (TMR_ticks_struct const * const pt_timeout)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_CheckTimeout                                           //
//                 - Check if a given timeout has expired                     //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  BOOL             expired ;
  TMR_ticks_struct currentTime ;

  // Fetch the current time
  TMR_CurrentTicks (&currentTime) ;

  if (currentTime.t_ticksCalc.u16_msShort == pt_timeout->t_ticksCalc.u16_msShort)
  {
    expired = (currentTime.t_ticksCalc.u32_lsLong >= pt_timeout->t_ticksCalc.u32_lsLong) ?
              TRUE :
              FALSE ;
  }
  else
  {
    expired = (currentTime.t_ticksCalc.u16_msShort >= pt_timeout->t_ticksCalc.u16_msShort) ?
              TRUE :
              FALSE ;
  }

  return (expired) ;
}
// End: TMR_CheckTimeout


void TMR_SetTimeStamp (TMR_ticks_struct * const pt_timestamp)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_SetTimeStamp                                           //
//                 - Check if a given timeout has expired                     //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  // Fetch the current time
  TMR_CurrentTicks (pt_timestamp) ;

  return ;
}
// End: TMR_SetTimeStamp


unsigned long TMR_TimeStampAge (TMR_ticks_struct const * const pt_timestamp)
////////////////////////////////////////////////////////////////////////////////
// Function:       TMR_TimeStampAge                                           //
//                 - Calculate the time now and the time the give timestamp   //
//                   was set                                                  //
// History :       24 Jul 2004 by R. Delien:                                  //
//                 - Initial revision.                                        //
////////////////////////////////////////////////////////////////////////////////
{
  unsigned long       age         = 0 ;
  TMR_ticks_struct    currentTime ;

  // Fetch the current time
  TMR_CurrentTicks (&currentTime) ;

  // Check Most Significant Short for future time stamps
  if (currentTime.t_ticksCalc.u16_msShort >= pt_timestamp->t_ticksCalc.u16_msShort)
  {
    switch (currentTime.t_ticksCalc.u16_msShort - pt_timestamp->t_ticksCalc.u16_msShort)
    {
      case 0: // Most Significant Short is the same
        if (currentTime.t_ticksCalc.u32_lsLong > pt_timestamp->t_ticksCalc.u32_lsLong)
        {
          // Past time stamp: Just subtract the Least Significant LWords
          age = currentTime.t_ticksCalc.u32_lsLong - pt_timestamp->t_ticksCalc.u32_lsLong ;
        }
        else
        {
          // Future or current time stamp
          age = 0 ;
        }
        break ;

      case 1: // Most Significant LWord is increased just by 1
        if (currentTime.t_ticksCalc.u32_lsLong < pt_timestamp->t_ticksCalc.u32_lsLong)
        {
          // The Least Significant LWords has only wrapped around
          age = (0xFFFFFFFF - (pt_timestamp->t_ticksCalc.u32_lsLong - currentTime.t_ticksCalc.u32_lsLong)) + 1 ;
        }
        else
        {
          // The The stamp has just aged beyond the maximum
          age = 0xFFFFFFFF ;
        }
        break ;

      default: // Most Significant LWord is increased by two of more
        // The time stamp is ancient, older than 49 days!
        age = 0xFFFFFFFF ;
        break ;
    }
  }
  else
  {
    // Future time stamp
    age = 0 ;
  }

  return (age) ;
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
  TMR_ticks_struct timeoutTime ;

  // Set a timout
  TMR_SetTimeout (&timeoutTime, delay) ;

  // Wait for it to expire
  while (!TMR_CheckTimeout (&timeoutTime))
  {
    // Just wait...
    ;
  }

  return ;
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
  unsigned char            u8_old_tmr3_dr[2] ;
  unsigned short volatile *pu16_old_tmr3 = (unsigned short volatile *)&(u8_old_tmr3_dr[0]) ;

  unsigned char            u8_cur_tmr3_dr[2] ;
  unsigned short volatile *pu16_cur_tmr3 = (unsigned short volatile *)&(u8_cur_tmr3_dr[0]) ;

  unsigned int             u24_ticks_to_wait ;
  unsigned int             u24_ticks_waited = 0 ;

  // Store TMR3 value first, for highest accuracy
  u8_old_tmr3_dr[0] = TMR3_DR_L ; // Read the lower byte first, this will also latch the higher byte
  u8_old_tmr3_dr[1] = TMR3_DR_H ; // Read the latched higher byte

  // Ticks-per-us is 12.5. To keep the 0.5, we use ticks-per-10us which is 125.
  // To calculate the proper nr of ticks to wait, the delay must be multiplied
  // by 10 too.
  u24_ticks_to_wait = (delay_us * TICKS_PER_10_US) / 10 ;

  while (u24_ticks_waited < u24_ticks_to_wait)
  {
    // Fetch the current TMR3 value
    u8_cur_tmr3_dr[0] = TMR3_DR_L ; // Read the lower byte first, this will also latch the higher byte
    u8_cur_tmr3_dr[1] = TMR3_DR_H ; // Read the latched higher byte

    // Check if the timer has decreased and if it rolled under
    if (*pu16_cur_tmr3 < *pu16_old_tmr3)
    {
      // Timer has decreased, just add the difference
      u24_ticks_waited += (*pu16_old_tmr3 - *pu16_cur_tmr3) ;
    }
    else if (*pu16_cur_tmr3 > *pu16_old_tmr3)
    {
      // Timer has rolled under.
      u24_ticks_waited += *pu16_old_tmr3 + (TMR3_RELOAD_VALUE - *pu16_cur_tmr3) ;
    }

    *pu16_old_tmr3 = *pu16_cur_tmr3 ;
  }

  return ;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

static void getticks (unsigned char * const currTicks)
/******************************************************************************/
/* Function:	getticks:						      */
/*		- Fetch current ticks accurately, without stopping the timer  */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	volatile unsigned char		tempByte ;

	/* To prevent a roll-over while reading the separate bytes, we could
	 * disable the interrupts, but this would reduce the ticks accuracy.
	 * That's why we use a trick: */
	do
	{
		/* Temporarilly store the first roll-over dependent byte
		 * directly from the running clock */
		tempByte = TMR1H ;

		currTicks[0] = TMR1L;
		currTicks[2] = ticks[0];
		currTicks[3] = ticks[1];
		currTicks[4] = ticks[2];
		currTicks[5] = ticks[3];
		currTicks[1] = TMR1H;
		// Copy the contents from the running clock into the return buffer
		*currTicks_ptr = t_currentTicks ;
		
		// Check if the Least Significant Byte has rolled over during this operation,
		// having the incread a More Significant Byte and making the buffer invalid
		} while (currTicks_ptr->u8_ticksByte[1] != tempByte) ;
		
		return ;
}
// End: TMR_CurrentTicks
#endif


void interrupt timer1_isr (void)
/******************************************************************************/
/* Function:	Timer 1 interrupt service routine:			      */
/*		- Each interrupt will increment the 32-bit ticks counter.     */
/* History :	10 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
//	unsigned char	index = 0;

test++;
PORTC = test & 0x01;

//TMR1H = 0xF6;
//TMR1L = 0x3F;

	/* There's a neater way to increase the counter, but this is the
	   quickst. Since this occurs ever 10 millisecond, at we only have
	   4 MHz, we'll go for the quickest sollution. */
//	while (!++ticks[index]) {
//		if (++index >= (NR_OF_TICK_BYTES - 2)) {
//			/* Reset! */
//		}
//	}

	/* Reset the interrupt */
	TMR1IF = 0;

	return;
}
/* End: timer1_isr */
