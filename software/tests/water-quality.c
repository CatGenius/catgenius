#include <assert.h>
#include <stdio.h>

#include "../common/waterquality.h"

static void test_initial_and_confirmation(void)
{
	struct waterquality quality;
	unsigned char analog_bad, comparator_bad, i;

	assert(waterquality_init(&quality, 410));
	assert(waterquality_begin(&quality, 1));
	waterquality_batch(&quality, 410, 1);
	assert(quality.status == WATER_QUALITY_GOOD);
	assert(waterquality_begin(&quality, 0));
	waterquality_batch(&quality, 410, 0);
	assert(quality.status == WATER_QUALITY_GOOD);
	for (analog_bad = 0; analog_bad <= 10; analog_bad++)
		for (comparator_bad = 0; comparator_bad <= 10; comparator_bad++) {
			assert(waterquality_init(&quality, 410));
			assert(waterquality_begin(&quality, 1));
			waterquality_batch(&quality, 411, 0);
			assert(!waterquality_begin(&quality, 0));
			for (i = 0; i < 10; i++) {
				assert(quality.status == WATER_QUALITY_CHECKING);
				waterquality_batch(&quality, i < analog_bad ? 411 : 410,
						   i >= comparator_bad);
			}
			assert(quality.status == (comparator_bad >= 6 ? WATER_QUALITY_LEVEL :
				(analog_bad >= 6 ? WATER_QUALITY_OPTICAL : WATER_QUALITY_GOOD)));
		}
	/* Comparator samples cannot create a fault in analogue-only checks. */
	assert(waterquality_init(&quality, 410));
	assert(waterquality_begin(&quality, 0));
	waterquality_batch(&quality, 411, 0);
	for (i = 0; i < 10; i++)
		waterquality_batch(&quality, 410, 0);
	assert(quality.status == WATER_QUALITY_GOOD);
}

static void optical_fault(struct waterquality *quality)
{
	unsigned char i;

	assert(waterquality_begin(quality, 0));
	for (i = 0; i < 11; i++)
		waterquality_batch(quality, quality->threshold + 1, 0);
	assert(quality->status == WATER_QUALITY_OPTICAL);
}

static void test_recovery_and_history(void)
{
	struct waterquality quality;
	unsigned char i;
	unsigned int threshold;

	for (threshold = 410; threshold <= 600; threshold += 190) {
		assert(waterquality_init(&quality, threshold));
		optical_fault(&quality);
		assert(quality.saved_mean == threshold + 1);
		assert(!waterquality_begin(&quality, 1));
		for (i = 0; i < 3; i++)
			waterquality_batch(&quality, threshold - 1, 0);
		waterquality_batch(&quality, threshold, 0);
		assert(quality.good == 0 && quality.status == WATER_QUALITY_OPTICAL);
		for (i = 0; i < 4; i++)
			waterquality_batch(&quality, threshold - 1, 0);
		assert(quality.status == WATER_QUALITY_GOOD);
		assert(waterquality_begin(&quality, 0));
		waterquality_batch(&quality, threshold + 1, 0);
		for (i = 0; i < 10; i++)
			waterquality_batch(&quality, threshold - 205, 0);
		assert(quality.status == WATER_QUALITY_OPTICAL);
		for (i = 0; i < 4; i++)
			waterquality_batch(&quality, 0, 0);
		assert(waterquality_begin(&quality, 0));
		waterquality_batch(&quality, threshold + 1, 0);
		for (i = 0; i < 10; i++)
			waterquality_batch(&quality, threshold - 206, 0);
		assert(quality.status == WATER_QUALITY_GOOD);
	}
	assert(!waterquality_init(&quality, 205));
	assert(!waterquality_init(&quality, 1024));
	assert(waterquality_init(&quality, 206));
	assert(waterquality_init(&quality, 1023));
}

static void test_last_mean_and_fault_precedence(void)
{
	struct waterquality quality;
	unsigned char i;

	assert(waterquality_init(&quality, 410));
	assert(waterquality_begin(&quality, 1));
	waterquality_batch(&quality, 1023, 1);
	for (i = 0; i < 10; i++)
		waterquality_batch(&quality, i < 6 ? 1023 : 100, 1);
	assert(quality.status == WATER_QUALITY_OPTICAL);
	assert(quality.saved_mean == 100);
	for (i = 0; i < 4; i++)
		waterquality_batch(&quality, 0, 0);
	assert(waterquality_begin(&quality, 1));
	waterquality_batch(&quality, 1023, 1);
	for (i = 0; i < 10; i++)
		waterquality_batch(&quality, 205, 1);
	assert(quality.status == WATER_QUALITY_GOOD);
	assert(waterquality_begin(&quality, 1));
	for (i = 0; i < 11; i++)
		waterquality_batch(&quality, 1023, 0);
	assert(quality.status == WATER_QUALITY_LEVEL);
	assert(quality.saved_mean == 100);
	waterquality_batch(&quality, 0, 1);
	assert(quality.status == WATER_QUALITY_LEVEL);
}

int main(void)
{
	test_initial_and_confirmation();
	test_recovery_and_history();
	test_last_mean_and_fault_precedence();
	puts("Water-quality decision checks passed.");
	return 0;
}
