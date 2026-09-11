/* Optical-quality decisions, independent of acquisition and actuator timing. */
#ifndef WATERQUALITY_H
#define WATERQUALITY_H

#ifndef WATER_QUALITY_THRESHOLD
#define WATER_QUALITY_THRESHOLD	410
#endif
#if (WATER_QUALITY_THRESHOLD <= 205) || (WATER_QUALITY_THRESHOLD > 1023)
#error Water quality threshold must be in the range 206..1023
#endif

#define WATER_QUALITY_UNCHECKED	0
#define WATER_QUALITY_CHECKING	1
#define WATER_QUALITY_GOOD	2
#define WATER_QUALITY_OPTICAL	3
#define WATER_QUALITY_LEVEL	4

#define WATER_QUALITY_REPEATS	10
#define WATER_QUALITY_BAD	6
#define WATER_QUALITY_RECOVERY	4
#define WATER_QUALITY_MARGIN	205

struct waterquality {
	unsigned int	threshold;
	unsigned int	saved_mean;
	unsigned char	status;
	unsigned char	probe;
	unsigned char	repeats;
	unsigned char	analog_bad;
	unsigned char	comparator_bad;
	unsigned char	good;
};

unsigned char	waterquality_init	(struct waterquality *quality,
					 unsigned int threshold);
unsigned char	waterquality_begin	(struct waterquality *quality,
					 unsigned char probe);
void		waterquality_batch	(struct waterquality *quality,
					 unsigned int mean,
					 unsigned char comparator_high);

#endif
