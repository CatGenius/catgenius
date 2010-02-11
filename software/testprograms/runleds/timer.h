/******************************************************************************/
/* File    :	timer.h							      */
/* Function:	Include file of 'timer.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef TIMER_H				/* Include file already compiled? */
#define TIMER_H

#ifdef TIMER_C				/* Compiled in timer.C? */
#define extern
#else
#ifdef __cplusplus			/* Compiled for C++? */
#define extern extern "C"
#else
#define extern extern
#endif // __cplusplus
#endif // TIMER_C

#define KHZ		(1000UL)
#define MHZ		(1000UL * (KHZ))
#define FOSC		(4UL * MHZ)		/* Crystal frequency */

#define SECOND		(((FOSC)/4)/8)		/* Number of timer ticks per second */
#define MINUTE		(60 * (SECOND))		/* Number of timer ticks per minute */
#define HOUR		(60 * (MINUTE))		/* Number of timer ticks per hour */

struct timer {
	unsigned short	timer1 ;
	unsigned long	overflows ;
};

void		timer_init		(void) ;

void		timer_term		(void) ;

void		settimeout		(struct timer		* const timer_p,
					 unsigned long		  const timout) ;

void		PostponeTimeout		(struct timer		* const timer_p,
					 unsigned long		  const postpone) ;

void		ExpireTimeout		(struct timer		* const timer_p) ;

void		NeverTimeout		(struct timer		* const timer_p) ;

unsigned char	timeoutexpired		(struct timer	 const	* const timer_p) ;

void		SetTimeStamp		(struct timer		* const timer_p) ;

unsigned long	TimeStampAge		(struct timer	 const	* const timer_p) ;

void		Delay			(unsigned long		* const delay) ;

void		MicroDelay		(unsigned short		  const delay_us) ;

#endif /* TIMER_H */
