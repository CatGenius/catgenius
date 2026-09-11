/* Uses the register/timer fixture in state-machines.c. */
#ifdef WATERSENSOR_ANALOG
static void prepare_probe(unsigned int value)
{
	unsigned char i;

	start_analog_cycle();
	for (i = 0; i < 4; i++) {
		ticks = sensortimer.overflows;
		water_work();
		assert(ADCON0bits.GO);
		ADRES = value;
		ADCON0bits.GO = 0;
		water_work();
	}
	assert(state == WAIT_PROBE && TMR4IE);
}

static void test_probe_lifecycle(void)
{
	unsigned char filling_before, i;
	unsigned int last;
	unsigned long deadline;

	for (filling_before = 0; filling_before <= 1; filling_before++) {
		reset_firmware();
		water_fill(filling_before);
		prepare_probe(22);
		complete_probe(1);
		/* No main-loop work is needed to end a non-filling probe. */
		assert(!!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK) == filling_before);
		assert(!!(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK) == filling_before);
		ticks += SECOND;
		water_work();
		assert(!water_failed());
		assert(water_comparator_valid() && water_comparator());
		assert(water_reflectionquality() == 22);
		assert(water_reflection_filling() == filling_before);
	}
	reset_firmware();
	qualify_water(0);
	water_fill(1);
	prepare_probe(1023);
	deadline = sensortimer.overflows;
	ticks = deadline - 1;
	water_work();
	assert(!water_failed() && TMR4IE);
	ticks++;
	water_work();
	assert(water_failed() && !water_valid() && !water_comparator_valid());
	assert(water_acquisition_fault() == WATER_ACQUISITION_PROBE);
	assert(!TMR4IE && !T4CON && !TMR4IF);
	assert(water_reflectionquality() == 0);
	assert_stopped_outputs();
	water_fill(1);
	assert(!water_filling());
	qualify_water(0);
	assert(water_acquisition_fault() == WATER_ACQUISITION_OK);
	assert(!water_filling());

	/* Cancellation at every conversion boundary discards the partial batch. */
	for (i = 0; i < 4; i++) {
		unsigned char sample;
		reset_firmware();
		sample_water(22);
		last = water_reflectionquality();
		start_analog_cycle();
		for (sample = 0; sample <= i; sample++) {
			ticks = sensortimer.overflows;
			water_work();
			assert(ADCON0bits.GO);
			if (sample < i) {
				ADRES = 1023;
				ADCON0bits.GO = 0;
				water_work();
			}
		}
		water_fill(1);
		assert(!ADCON0bits.GO && !TMR4IE && state == LED_ON);
		assert(water_reflectionquality() == last);
		sample_water(24);
		assert(water_reflectionquality() == 24 && water_reflection_filling());
	}
	reset_firmware();
	prepare_probe(22);
	TMR4IF = 1;
	water_fill(0);
	assert(!TMR4IE && !TMR4IF && !T4CON && !probe_done);
	assert_stopped_outputs();
	water_ledalwayson(1);
	prepare_probe(22);
	complete_probe(1);
	assert(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK);
	assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
	water_work();
	reset_firmware();
	GIE = 0;
	prepare_probe(22);
	assert(!GIE);
	water_check_cancel();
	assert(!GIE && !TMR4IE && !T4CON);
}

static void test_analog_level_and_mean(void)
{
	unsigned char i;
	unsigned long started;

	reset_firmware();
	assert(ADCON0bits.CHS == 1 && ADCON1bits.ADFM && ADCON1bits.ADCS == 2);
	start_analog_cycle();
	started = ticks;
	ticks = sensortimer.overflows - 1;
	water_work();
	assert(!ADCON0bits.GO && state == START_CONVERSION);
	assert(sensortimer.overflows - started == QUALITY_SETTLE);
	water_fill(0);
	sample_analog(0, 1, 2, 4, 1);
	assert(water_reflectionquality() == 1);
	assert(!water_reflection_filling());
	/* A high ADC mean is not a high-water decision or a quality diagnosis. */
	for (i = 0; i < HYSTERESIS_MAX; i++)
		sample_analog(1023, 1023, 1023, 1023, 1);
	assert(water_valid() && !water_detected() && !water_failed());
	assert(water_reflectionquality() == 1023);
	assert(water_quality() == WATER_QUALITY_UNCHECKED);
	for (i = 0; i < HYSTERESIS_MAX - 1; i++)
		sample_analog(22, 22, 22, 22, 0);
	sample_analog(22, 22, 22, 22, 1);
	for (i = 0; i < HYSTERESIS_MAX - 1; i++)
		sample_analog(22, 22, 22, 22, 0);
	assert(!water_detected());
	sample_analog(22, 22, 22, 22, 0);
	assert(water_detected());
}

static void test_quality_acquisition(void)
{
	unsigned char i;

	reset_firmware();
	assert(water_check(1));
	water_fill(1);
	assert(!water_filling());
	sample_analog(409, 410, 410, 411, 1);
	assert(water_quality() == WATER_QUALITY_GOOD && !water_valid());
	qualify_water(0);
	assert(water_check(1));
	for (i = 0; i < 11; i++) {
		assert(water_quality() == WATER_QUALITY_CHECKING);
		sample_analog(600, 600, 600, 600, 1);
		assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
	}
	assert(water_quality() == WATER_QUALITY_OPTICAL);
	assert(!water_check(1));
	for (i = 0; i < 3; i++) {
		sample_analog(409, 409, 409, 409, 0);
		assert(!TMR4IE && !probe_done);
		assert(water_quality() == WATER_QUALITY_OPTICAL);
	}
	sample_analog(410, 410, 410, 410, 0);
	assert(quality.good == 0);
	for (i = 0; i < 4; i++)
		sample_analog(409, 409, 409, 409, 0);
	assert(water_quality() == WATER_QUALITY_GOOD && !water_valid());
	assert_stopped_outputs();
	qualify_water(0);
	assert(water_check(1));
	for (i = 0; i < 11; i++)
		sample_analog(600, 600, 600, 600, 0);
	assert(water_quality() == WATER_QUALITY_LEVEL);
	for (i = 0; i < 5; i++)
		sample_analog(0, 0, 0, 0, 1);
	assert(water_quality() == WATER_QUALITY_LEVEL);
	/* A latched level fault plus an acquisition fault must remain retryable. */
	fail_conversion();
	assert(water_failed());
	assert(water_check(1));
	sample_analog(22, 22, 22, 22, 1);
	assert(water_quality() == WATER_QUALITY_GOOD);
	assert(water_failed());
	qualify_water(0);
	assert(!water_failed() && !water_filling());

	reset_firmware();
	assert(water_check(0));
	sample_analog(410, 410, 410, 410, 0);
	assert(water_quality() == WATER_QUALITY_GOOD && !water_comparator_valid());
	assert(!TMR4IE && !probe_done);
	assert_stopped_outputs();
	assert(water_check(1));
	prepare_probe(22);
	water_check_cancel();
	assert(water_quality() == WATER_QUALITY_UNCHECKED);
	assert_stopped_outputs();
	assert(!TMR4IE && !T4CON);
	set_Dryer(1);
	assert(!water_check(1));
	set_Dryer(0);
	assert(water_check(1));
	begin_analog_conversion();
	ticks = sensortimer.overflows;
	water_work();
	assert(water_acquisition_fault() == WATER_ACQUISITION_ADC);
	assert(!quality.repeats && !quality.good);
	assert_stopped_outputs();
}
#endif
