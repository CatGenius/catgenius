/* Exercise RTC rollover and setting helpers with supplied timer deadlines. */
#include <assert.h>
#include <stdio.h>
#include "../common/rtc.c"

static unsigned long ticks;
void settimeout(struct timer * const timer_p, unsigned long const duration)
{
	timer_p->overflows = ticks + duration;
}
void postponetimeout(struct timer * const timer_p, unsigned long const duration)
{
	timer_p->overflows += duration;
}
unsigned char timeoutexpired(struct timer const * const timer_p)
{
	return ticks >= timer_p->overflows;
}

int main(void)
{
	rtc_init(POWER_FAILURE);
	assert(!currenttime.seconds && !currenttime.minutes && !currenttime.hours);
	currenttime.seconds = currenttime.minutes = 59;
	currenttime.hours = 23;
	currenttime.weekday = 6;
	ticks = SECOND;
	rtc_work();
	assert(uptime == 1 && !currenttime.seconds && !currenttime.minutes);
	assert(!currenttime.hours && !currenttime.weekday);
	/* A delayed worker catches up without resetting the second deadline. */
	ticks += 3 * SECOND;
	rtc_work(); rtc_work(); rtc_work(); rtc_work();
	assert(uptime == 4 && currenttime.seconds == 3);
	currenttime.minutes = 59;
	incminutes();
	assert(!currenttime.minutes);
	currenttime.weekday = 6;
	incweekday();
	assert(!currenttime.weekday);
	puts("Host RTC checks passed.");
	return 0;
}
