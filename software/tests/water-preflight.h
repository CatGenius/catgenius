/* Program integration checks using the supplied wash/cleanup instructions. */
static void test_program_start_compatibility(void)
{
	reset_firmware();
	litterlanguage_start(0);
	litterlanguage_work();
	assert(ins_state == STATE_FETCH_INS && ins_pointer == washprogram + 1);
	litterlanguage_work();
	assert(ins_pointer == washprogram + 2 && !get_Pump());

	reset_firmware();
	nvram[NVM_BOXSTATE] = BOX_WET;
	litterlanguage_init(0);
	litterlanguage_work();
	assert(ins_state == STATE_FETCH_INS && ins_pointer == cleanupprogram + 1);
	litterlanguage_work();
	assert(get_Pump());
	assert(nvram[NVM_BOXSTATE] == BOX_WET);
#ifndef WATERSENSOR_ANALOG
	reset_firmware();
	litterlanguage_start(1);
	litterlanguage_work();
	assert(ins_state == STATE_FETCH_INS);
	litterlanguage_work();
	assert(get_Pump());
#endif
}

#ifdef WATERSENSOR_ANALOG
static void start_preflight(void)
{
	litterlanguage_start(1);
	litterlanguage_work();
	assert(ins_state == STATE_CHECK_WATER && ins_pointer == washprogram + 1);
	assert_stopped_outputs();
	litterlanguage_work();
	assert(check_started && water_quality() == WATER_QUALITY_CHECKING);
}

static void pass_preflight(void)
{
	unsigned char i;

	sample_analog(22, 22, 22, 22, 1);
	litterlanguage_work();
	assert(ins_state == STATE_CHECK_WATER);
	assert_stopped_outputs();
	for (i = 0; i < HYSTERESIS_MAX; i++) {
		sample_water(0);
		litterlanguage_work();
		assert_stopped_outputs();
	}
	assert(ins_state == STATE_FETCH_INS && ins_pointer == washprogram + 1);
	assert(timeoutneverexpires(&timer_waitins));
	litterlanguage_work();
	assert(ins_pointer == washprogram + 2 && get_Pump());
	assert(nvram[NVM_BOXSTATE] >= BOX_MESSY);
}

static void test_preflight_success_and_timeout(void)
{
	unsigned long deadline, remaining;

	reset_firmware();
	start_preflight();
	pass_preflight();

	reset_firmware();
	start_preflight();
	deadline = timer_waitins.overflows;
	ticks = deadline - 1;
	litterlanguage_work();
	assert(ins_state == STATE_CHECK_WATER && !error_execution);
	ticks++;
	litterlanguage_work();
	assert(ins_state == STATE_IDLE && error_execution);
	assert(water_quality() == WATER_QUALITY_UNCHECKED);
	assert(nvram[NVM_BOXSTATE] == BOX_TIDY);
	assert_stopped_outputs();
	assert(timeoutneverexpires(&timer_waitins));

	reset_firmware();
	start_preflight();
	remaining = timer_waitins.overflows - ticks;
	litterlanguage_pause(1);
	sample_analog(22, 22, 22, 22, 1);
	qualify_water(0);
	ticks += 10 * SECOND;
	litterlanguage_work();
	assert(paused && ins_state == STATE_CHECK_WATER);
	assert_stopped_outputs();
	litterlanguage_pause(0);
	assert(timer_waitins.overflows == ticks + remaining);
	litterlanguage_work();
	assert(ins_state == STATE_FETCH_INS && !get_Pump());

	reset_firmware();
	start_preflight();
	prepare_probe(22);
	litterlanguage_stop();
	assert(!TMR4IE && !T4CON && !TMR4IF);
	assert(water_quality() == WATER_QUALITY_UNCHECKED);
	assert(nvram[NVM_BOXSTATE] == BOX_TIDY);
	assert_stopped_outputs();
	qualify_water(0);
	litterlanguage_work();
	assert(ins_state == STATE_IDLE);
}

static void test_preflight_quality_faults(void)
{
	unsigned char optic, was_paused, i;

	for (optic = 0; optic <= 1; optic++)
		for (was_paused = 0; was_paused <= 1; was_paused++) {
			reset_firmware();
			nvram[NVM_BOXSTATE] = BOX_WET;
			start_preflight();
			if (was_paused)
				litterlanguage_pause(1);
			for (i = 0; i < 11; i++)
				sample_analog(600, 600, 600, 600, optic);
			litterlanguage_work();
			assert(ins_state == STATE_IDLE && !paused && error_execution);
			assert(events[EVENT_ERR_EXECUTION][1] == 1);
			assert(nvram[NVM_BOXSTATE] == BOX_WET);
			assert_stopped_outputs();
			for (i = 0; i < 4; i++)
				sample_analog(0, 0, 0, 0, 1);
			assert(water_quality() == (optic ? WATER_QUALITY_GOOD : WATER_QUALITY_LEVEL));
			litterlanguage_work();
			assert(ins_state == STATE_IDLE);
			start_preflight();
			pass_preflight();
		}
	/* A new optical fault request cannot clear the strict recovery requirement. */
	reset_firmware();
	start_preflight();
	for (i = 0; i < 11; i++)
		sample_analog(600, 600, 600, 600, 1);
	litterlanguage_work();
	litterlanguage_start(1);
	litterlanguage_work();
	litterlanguage_work();
	assert(ins_state == STATE_IDLE && water_quality() == WATER_QUALITY_OPTICAL);
	assert(nvram[NVM_BOXSTATE] == BOX_TIDY);
	assert_stopped_outputs();

	/* Good quality alone does not authorize starting with newly detected water. */
	reset_firmware();
	start_preflight();
	sample_analog(22, 22, 22, 22, 1);
	qualify_water(600);
	litterlanguage_work();
	assert(ins_state == STATE_IDLE && error_execution);
	assert_stopped_outputs();
}

static void test_preflight_acquisition_faults(void)
{
	unsigned char i;

	reset_firmware();
	start_preflight();
	fail_conversion();
	litterlanguage_work();
	assert(ins_state == STATE_IDLE && error_execution);
	assert(water_quality() == WATER_QUALITY_UNCHECKED);
	assert_stopped_outputs();
	qualify_water(0);
	litterlanguage_work();
	assert(ins_state == STATE_IDLE);

	reset_firmware();
	assert(water_check(1));
	for (i = 0; i < 11; i++)
		sample_analog(600, 600, 600, 600, 0);
	fail_conversion();
	assert(water_quality() == WATER_QUALITY_LEVEL && water_failed());
	litterlanguage_start(1);
	assert(ins_state == STATE_IDLE && error_execution);
	assert(water_quality() == WATER_QUALITY_CHECKING);
	sample_analog(22, 22, 22, 22, 1);
	qualify_water(0);
	litterlanguage_work();
	assert(ins_state == STATE_IDLE && !water_failed());
	start_preflight();
	pass_preflight();
}

static void test_probe_program_faults(void)
{
	unsigned char was_paused;

	for (was_paused = 0; was_paused <= 1; was_paused++) {
		reset_firmware();
		qualify_water(0);
		instruction(INS_WAITTIME, 1000);
		nvram[NVM_BOXSTATE] = BOX_WET;
		set_Bowl(BOWL_CW);
		set_Arm(ARM_DOWN);
		water_fill(1);
		set_Pump(1);
		set_Dosage(1);
		set_Dryer(1);
		if (was_paused)
			litterlanguage_pause(1);
		prepare_probe(1023);
		ticks = sensortimer.overflows;
		water_work();
		litterlanguage_work();
		assert(ins_state == STATE_IDLE && !paused && error_execution);
		assert(water_acquisition_fault() == WATER_ACQUISITION_PROBE);
		assert(nvram[NVM_BOXSTATE] == BOX_WET);
		assert_stopped_outputs();
		qualify_water(0);
		litterlanguage_work();
		assert(ins_state == STATE_IDLE);
	}
}
#endif
