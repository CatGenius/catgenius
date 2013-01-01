/******************************************************************************/
/* File    :	rtc.h							      */
/* Function:	Include file of 'rtc.c'.				      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/******************************************************************************/

#ifndef RTC_H				/* Include file already compiled? */
#define RTC_H

struct time {
	unsigned char	seconds;
	unsigned char	minutes;
	unsigned char	hours;
	unsigned char	weekday;
};

/* Generic */
void		rtc_init		(unsigned char		  const flags) ;
void		rtc_work		(void) ;

void		printtime		(void) ;
void		incminutes		(void) ;
void		inchours		(void) ;
void		incweekday		(void) ;

#endif /* RTC_H */
