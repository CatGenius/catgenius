/******************************************************************************/
/* File    :	rtc.c							      */
/* Function:	Real Time Clock						      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	3 Sep 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "rtc.h"
#include "timer.h"
#include "serial.h"


/******************************************************************************/
/* Macros								      */
/******************************************************************************/


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	second;
static unsigned long	uptime = 0;
static struct time	currenttime;


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/


/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void rtc_init (unsigned char flags)
/******************************************************************************/
/* Function:	rtc_init						      */
/*		- Initialize the real time clock			      */
/* History :	3 Sep 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	settimeout(&second, SECOND);

	if (flags & POWER_FAILURE) {
		currenttime.seconds = 0;
		currenttime.minutes = 0;
		currenttime.hours   = 0;
		currenttime.weekday = 0;
	}
}
/* rtc_init */


void rtc_work (void)
/******************************************************************************/
/* Function:	rtc_work						      */
/*		- Worker function for the real time clock		      */
/* History :	3 Sep 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
	if (timeoutexpired(&second)) {
		postponetimeout(&second, SECOND);
		uptime++;
		if (++currenttime.seconds >= 60) {
			currenttime.seconds = 0;
			if (++currenttime.minutes >=60) {
				currenttime.minutes = 0;
				if (++currenttime.hours >=24) {
					currenttime.hours = 0;
					if (++currenttime.weekday >=7)
						currenttime.weekday = 0;
				}
			}
		}
	}
}
/* rtc_work */


void printtime (void)
{
	TX5( "%d %d:%.2d.%.2d ",
		currenttime.weekday, currenttime.hours,
		currenttime.minutes, currenttime.seconds );
}


void incminutes (void)
{
	settimeout(&second, SECOND);
	if (++currenttime.minutes >=60)
		currenttime.minutes = 0;
	printtime();
}


void inchours (void)
{
	settimeout(&second, SECOND);
	if (++currenttime.hours >=23)
		currenttime.hours = 0;
}


void incweekday (void)
{
	settimeout(&second, SECOND);
	if (++currenttime.weekday >=7)
		currenttime.weekday = 0;
}


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

