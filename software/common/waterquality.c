/* Confirm optical-quality and comparator faults from completed ADC batches. */
#include "waterquality.h"

unsigned char waterquality_init (struct waterquality *quality,
				 unsigned int threshold)
{
	quality->threshold = WATER_QUALITY_THRESHOLD;
	quality->saved_mean = 0;
	quality->status = WATER_QUALITY_UNCHECKED;
	quality->probe = 0;
	quality->repeats = 0;
	quality->analog_bad = 0;
	quality->comparator_bad = 0;
	quality->good = 0;
	if ((threshold <= WATER_QUALITY_MARGIN) || (threshold > 1023))
		return 0;
	quality->threshold = threshold;
	return 1;
}

unsigned char waterquality_begin (struct waterquality *quality,
				  unsigned char probe)
{
	/* A new request must not bypass optical-fault recovery or reset a check. */
	if ((quality->status == WATER_QUALITY_CHECKING) ||
	    (quality->status == WATER_QUALITY_OPTICAL))
		return 0;
	quality->status = WATER_QUALITY_CHECKING;
	quality->probe = probe ? 1 : 0;
	quality->repeats = 0;
	quality->analog_bad = 0;
	quality->comparator_bad = 0;
	quality->good = 0;
	return 1;
}

void waterquality_batch (struct waterquality *quality, unsigned int mean,
			 unsigned char comparator_high)
{
	if (quality->status == WATER_QUALITY_OPTICAL) {
		/* Equality is not sufficient to clear an existing optical fault. */
		if (mean < quality->threshold) {
			if (++quality->good >= WATER_QUALITY_RECOVERY) {
				quality->status = WATER_QUALITY_GOOD;
				quality->good = 0;
			}
		} else
			quality->good = 0;
		return;
	}
	if (quality->status != WATER_QUALITY_CHECKING)
		return;
	if (!quality->repeats) {
		/* The first batch can pass; otherwise confirm with ten new batches. */
		if ((mean <= quality->threshold) &&
		    (!quality->probe || comparator_high))
			quality->status = WATER_QUALITY_GOOD;
		else
			quality->repeats = WATER_QUALITY_REPEATS;
		return;
	}
	if (mean > quality->threshold)
		quality->analog_bad++;
	if (quality->probe && !comparator_high)
		quality->comparator_bad++;
	if (--quality->repeats)
		return;

	/* Comparator failure takes precedence over poor optical reflection. */
	if (quality->comparator_bad >= WATER_QUALITY_BAD)
		quality->status = WATER_QUALITY_LEVEL;
	else if (quality->analog_bad >= WATER_QUALITY_BAD) {
		quality->status = WATER_QUALITY_OPTICAL;
		quality->saved_mean = mean;
	} else if ((quality->saved_mean >= quality->threshold) &&
		   (mean >= quality->threshold - WATER_QUALITY_MARGIN))
		quality->status = WATER_QUALITY_OPTICAL;
	else
		quality->status = WATER_QUALITY_GOOD;
}
