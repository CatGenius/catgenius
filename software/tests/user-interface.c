/* Exercise the actual UI; no interrupts, EEPROM or actuators are operated. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../catgenius/userinterface.c"

static unsigned long ticks;
static unsigned char nvram[4], running, is_paused, started_wet;
static unsigned int starts, stops, pauses, key_beeps;
static unsigned char generate_on_pause;

unsigned char eeprom_read(unsigned char address) { return nvram[address]; }
void eeprom_write(unsigned char address, unsigned char value) { nvram[address] = value; }
void printtime(void) {}
void set_LED(unsigned char number, unsigned char on) { (void)number; (void)on; }
void set_LED_Error(unsigned char pattern, unsigned char repeat) { (void)pattern; (void)repeat; }
void set_LED_Cartridge(unsigned char pattern, unsigned char repeat) { (void)pattern; (void)repeat; }
void set_LED_Cat(unsigned char pattern, unsigned char repeat) { (void)pattern; (void)repeat; }
void set_LED_Locked(unsigned char pattern, unsigned char repeat) { (void)pattern; (void)repeat; }
void set_Beeper(unsigned char pattern, unsigned char repeat) { (void)pattern; (void)repeat; }
void key_Beep(unsigned char beeps) { key_beeps += beeps; }
void timeoutnow(struct timer * const timer_p) { timer_p->timer1 = 0; timer_p->overflows = ticks; }
void timeoutnever(struct timer * const timer_p) { timer_p->timer1 = 0xffff; timer_p->overflows = 0xffffffffUL; }
void settimeout(struct timer * const timer_p, unsigned long const duration)
{
	timer_p->timer1 = 0;
	timer_p->overflows = ticks + duration;
}
void postponetimeout(struct timer * const timer_p, unsigned long const duration) { timer_p->overflows += duration; }
unsigned char timeoutexpired(struct timer const * const timer_p)
{
	return timer_p->timer1 != 0xffff && timer_p->overflows <= ticks;
}
void litterlanguage_start(unsigned char wet) { starts++; started_wet = wet; running = 1; }
unsigned char litterlanguage_running(void) { return running; }
unsigned char litterlanguage_paused(void) { return is_paused; }
void litterlanguage_pause(unsigned char pause)
{
	pauses++;
	is_paused = pause;
	if (generate_on_pause) {
		generate_on_pause = 0;
		litterlanguage_event(EVENT_ERR_EXECUTION, 1);
	}
}
void litterlanguage_stop(void) { stops++; running = is_paused = 0; }

static void reset_ui(unsigned char lock)
{
	/* Finish any pending work before resetting the fixture. */
	userinterface_work();
	userinterface_work();
	ticks = 0;
	memset(nvram, 0, sizeof(nvram));
	nvram[NVM_KEYUNDLOCK] = lock ? 0 : 0xff;
	running = is_paused = generate_on_pause = 0;
	starts = stops = pauses = key_beeps = 0;
	buttonmask_cur = buttonmask_cum = buttonmask_evt = 0;
	longhandled = cat_present = cat_detected = full_wash = 0;
	state = STATE_IDLE;
	panel_mode = PANEL_AUTOMODE;
	error_nr = 0;
	timeoutnever(&holdtimeout);
	timeoutnever(&cartridgetimeout);
	userinterface_init(0);
}

static void test_buttons(void)
{
	reset_ui(0);
	startbutton_event(0);
	startbutton_event(1);
	userinterface_work();
	assert(starts == 1 && started_wet && running);
	startbutton_event(0);
	startbutton_event(1);
	userinterface_work();
	assert(is_paused && starts == 1);

	reset_ui(0);
	startbutton_event(0);
	ticks += HOLDTIME;
	userinterface_work();
	assert(starts == 1 && !started_wet);
	startbutton_event(1);
	userinterface_work();
	assert(starts == 1 && !is_paused);

	reset_ui(1);
	startbutton_event(0);
	startbutton_event(1);
	setupbutton_event(0);
	setupbutton_event(1);
	userinterface_work();
	assert(!starts && auto_mode == AUTO_MANUAL && !key_beeps);
	startbutton_event(0);
	setupbutton_event(0);
	ticks += HOLDTIME;
	userinterface_work();
	assert(!locked && nvram[NVM_KEYUNDLOCK]);
	startbutton_event(1);
	setupbutton_event(1);
	userinterface_work();
	assert(!starts && auto_mode == AUTO_MANUAL);
}

static void test_deferred_faults(void)
{
	reset_ui(0);
	running = 1;
	generate_on_pause = 1;
	litterlanguage_event(EVENT_ERR_DRAINING, 1);
	assert(!pauses && !stops);
	userinterface_work();
	assert(pauses == 1 && !stops && error_nr == EVENT_ERR_DRAINING);
	userinterface_work();
	assert(stops == 1 && error_nr == EVENT_ERR_EXECUTION);
	litterlanguage_event(EVENT_ERR_EXECUTION, 0);
	userinterface_work();
	assert(!error_nr);
}

static void test_locked_holds(void)
{
	unsigned char active, button;
	for (active = 0; active <= 1; active++)
		for (button = START_BUTTON; button <= SETUP_BUTTON; button++) {
			reset_ui(1);
			running = active;
			process_button(button, 1);
			ticks += HOLDTIME;
			userinterface_work();
			assert(locked && running == active);
			assert(!starts && !stops && !pauses && !key_beeps);
			process_button(button, 0);
			userinterface_work();
			assert(!starts && !stops && !pauses);
		}
}

static void test_fault_bursts(void)
{
	unsigned int i;
	unsigned char event;
	reset_ui(0);
	running = 1;
	for (i = 0; i < 100; i++)
		for (event = EVENT_ERR_FILLING; event <= EVENT_ERR_FLOOD; event++)
			litterlanguage_event(event, 1);
	userinterface_work();
	assert(stops == 1 && pauses == 3 && error_nr == EVENT_ERR_EXECUTION);
	for (event = EVENT_ERR_FILLING; event <= EVENT_ERR_FLOOD; event++)
		litterlanguage_event(event, 0);
	userinterface_work();
	assert(!error_nr && stops == 1 && pauses == 3);

	/* An assertion followed by clearing must still be acted upon once. */
	reset_ui(0);
	running = 1;
	for (i = 0; i < 100; i++) {
		litterlanguage_event(EVENT_ERR_OVERHEAT, 1);
		litterlanguage_event(EVENT_ERR_OVERHEAT, 0);
	}
	userinterface_work();
	assert(pauses == 1 && !error_nr);
	/* A last assertion takes precedence over an earlier clear. */
	litterlanguage_event(EVENT_ERR_OVERHEAT, 0);
	litterlanguage_event(EVENT_ERR_OVERHEAT, 255);
	userinterface_work();
	assert(pauses == 2 && error_nr == EVENT_ERR_OVERHEAT);
	litterlanguage_event(EVENT_ERR_OVERHEAT, 0);
	litterlanguage_event(EVENT_LEVEL_CHANGED, 100);
	litterlanguage_event(255, 1);
	userinterface_work();
	assert(!error_nr && pauses == 2);
}

int main(void)
{
	test_buttons();
	test_deferred_faults();
	test_locked_holds();
	test_fault_bursts();
	puts("Host UI checks passed.");
	return 0;
}
