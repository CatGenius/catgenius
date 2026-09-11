/* Cooperative application scenarios using separately linked firmware modules. */
#include <xc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/hardware.h"
#include "../common/timer.h"
#include "../common/rtc.h"
#include "../common/water.h"
#include "../common/cmdline.h"
#include "../catgenius/userinterface.h"
#include "../catgenius/litterlanguage.h"
#include "../catgenius/romwashprogram.h"
#undef printf

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define CHECK(condition) do { if (!(condition)) fail(#condition, __LINE__); } while (0)
#define TICKS_NEVER UINT64_C(0xffffffffffff)
#define BOX_TIDY 0
#define BOX_MESSY 1
#define BOX_WET 2
#define ACT_FILL 1
#define ACT_PUMP 2
#define ACT_DOSE 4
#define ACT_DRYER 8
#define ACT_BOWL 16
#define ACT_ARM 32
#define ACT_WET (ACT_FILL | ACT_PUMP | ACT_DOSE | ACT_DRYER)

volatile unsigned char PORTA, PORTB, PORTC, PORTD, PORTE;
#ifdef _16F1939
volatile unsigned char LATA, LATB, LATC, LATD, LATE;
#endif
volatile unsigned char TRISA, TRISB, TRISC, TRISD, TRISE;
volatile unsigned char ANSELA, ANSELB, ANSELD, ANSELE;
volatile unsigned char WPUB, WPUE, nWPUEN, nRBPU;
volatile unsigned char RBIF, RBIE, IOCBP, IOCBN, IOCBF, IOCIF, IOCIE;
volatile unsigned char ADCON1;
volatile unsigned int ADRES;
volatile unsigned char TMR4, PR4, T4CON, TMR4IE, TMR4IF, GIE;
volatile struct host_adcon0 ADCON0bits;
volatile struct host_adcon1 ADCON1bits;

/* This deliberately small recipe is NOT a replacement for ROM recipe tests.
 * It exercises all six actuators, direction state, waits and auto-dose timing.
 * No PIC-specific INS_CALL/INS_RETURN address conversion is executed. */
const struct instruction washprogram[] = {
	{INS_START, FLAGS_WETRUN | FLAGS_DRYRUN | INS_END},
	{INS_BOWL, BOWL_CCW},
	{INS_ARM, ARM_DOWN},
	{INS_WAITTIME, 500},
	{INS_ARM, ARM_STOP},
	{INS_WATER, 1},
	{INS_WAITWATER, 1},
	{INS_AUTODOSE, 2},
	{INS_WAITDOSAGE, 0},
	{INS_PUMP, 1},
	{INS_WAITWATER, 0},
	{INS_PUMP, 0},
	{INS_DRYER, 1},
	{INS_WAITTIME, 3000},
	{INS_DRYER, 0},
	{INS_BOWL, BOWL_STOP},
	{INS_END, 0}
};
const struct instruction cleanupprogram[] = {
	{INS_START, FLAGS_WETRUN | FLAGS_DRYRUN | INS_END},
	{INS_PUMP, 1},
	{INS_WAITWATER, 0},
	{INS_PUMP, 0},
	{INS_END, 0}
};
const struct command commands[] = {
	{"?", help}, {"help", help}, {"echo", echo}, {"", NULL}
};

static const char *scenario_name;
static uint64_t ticks;
static unsigned char nvram[256];
static unsigned int box_writes[3];
static const struct instruction *requested;
static const struct instruction *corrupt_instruction;
static unsigned char buttons, hot, water_high;
static unsigned char auto_fill = 1, auto_drain = 1;
static uint64_t fill_since = TICKS_NEVER, drain_since = TICKS_NEVER;
static uint64_t on_ticks[6];
static unsigned char seen_actuators;
static char firmware_output[4096];
static size_t output_used;

#ifdef WATERSENSOR_ANALOG
static uint64_t adc_due = TICKS_NEVER, probe_due = TICKS_NEVER;
static unsigned char hold_adc, hold_probe;
static unsigned char noisy_comparator;
static int adc_override = -1;
static unsigned int probes;
#endif

/* Changes, including changes inside a main-loop pass, are retained on failure.
 * Numbered LEDs are the observable UI result, not a mock of its error callback. */
struct snapshot {
	unsigned char actuators, bowl, arm, running, paused;
	unsigned char valid, high, failed, quality, leds, box;
};
struct trace_entry {
	uint64_t ticks;
	const char *worker;
	struct snapshot state;
};
static struct trace_entry trace[64];
static unsigned int trace_count;

static unsigned char actuators(void)
{
	return (water_filling() ? ACT_FILL : 0) |
		(get_Pump() ? ACT_PUMP : 0) | (get_Dosage() ? ACT_DOSE : 0) |
		(get_Dryer() ? ACT_DRYER : 0) | (get_Bowl() ? ACT_BOWL : 0) |
		(get_Arm() ? ACT_ARM : 0);
}

static unsigned char numbered_leds(void)
{
	return ((LED_1(LAT) & LED_1_MASK) ? 1 : 0) |
		((LED_2(LAT) & LED_2_MASK) ? 2 : 0) |
		((LED_3(LAT) & LED_3_MASK) ? 4 : 0) |
		((LED_4(LAT) & LED_4_MASK) ? 8 : 0);
}

static void record(const char *worker)
{
	struct snapshot s;
	struct trace_entry *entry;

	memset(&s, 0, sizeof(s));
	s.actuators = actuators();
	s.bowl = get_Bowl();
	s.arm = get_Arm();
	s.running = litterlanguage_running();
	s.paused = litterlanguage_paused();
	s.valid = water_valid();
	s.high = water_detected();
	s.failed = water_failed();
#ifdef WATERSENSOR_ANALOG
	s.quality = water_quality();
#endif
	s.leds = numbered_leds();
	s.box = nvram[NVM_BOXSTATE];
	seen_actuators |= s.actuators;
	if (trace_count && !memcmp(&s, &trace[(trace_count - 1) % COUNT(trace)].state,
				  sizeof(s)))
		return;
	entry = &trace[trace_count++ % COUNT(trace)];
	entry->ticks = ticks;
	entry->worker = worker;
	entry->state = s;
}

static void fail(const char *condition, unsigned int line)
{
	unsigned int i = trace_count > COUNT(trace) ? trace_count - COUNT(trace) : 0;

	fprintf(stderr, "%s:%u at %.3fs: %s\n", scenario_name, line,
		(double)ticks / SECOND, condition);
	fprintf(stderr, " time(s) worker       act bowl arm run pause valid high fail quality LEDs box\n");
	for (; i < trace_count; i++) {
		const struct trace_entry *e = &trace[i % COUNT(trace)];
		const struct snapshot *s = &e->state;
		fprintf(stderr, "%8.3f %-12s %02x %4u %3u %3u %5u %5u %4u %4u %7u %04x %3u\n",
			(double)e->ticks / SECOND, e->worker, s->actuators,
			s->bowl, s->arm, s->running, s->paused, s->valid,
			s->high, s->failed, s->quality, s->leds, s->box);
	}
	fprintf(stderr, "Recent firmware output:\n%s\n", firmware_output);
	exit(EXIT_FAILURE);
}

int integration_printf(const char *format, ...)
{
	va_list args;
	char message[512];
	int length;

	va_start(args, format);
	length = vsnprintf(message, sizeof(message), format, args);
	va_end(args);
	CHECK(length >= 0 && (size_t)length < sizeof(message));
	if (output_used + (size_t)length >= sizeof(firmware_output))
		output_used = 0;
	memcpy(firmware_output + output_used, message, (size_t)length + 1);
	output_used += (size_t)length;
	return length;
}

void putch(char c) { integration_printf("%c", c); }
unsigned char readch(char *c) { (void)c; return 0; }

unsigned char eeprom_read(unsigned char address) { return nvram[address]; }
void eeprom_write(unsigned char address, unsigned char value)
{
	nvram[address] = value;
	if (address == NVM_BOXSTATE) {
		CHECK(value < COUNT(box_writes));
		box_writes[value]++;
	}
}

void romwashprogram_reqins(const struct instruction *address)
{
	size_t i;

	/* Equality is defined even for unrelated pointers; never subtract/order them. */
	for (i = 0; i < COUNT(washprogram); i++)
		if (address == &washprogram[i]) {
			requested = address;
			return;
		}
	for (i = 0; i < COUNT(cleanupprogram); i++)
		if (address == &cleanupprogram[i]) {
			requested = address;
			return;
		}
	CHECK(0 && "instruction address outside supplied recipes");
}

unsigned char romwashprogram_getins(struct instruction * const instruction)
{
	CHECK(requested != NULL);
	*instruction = *requested;
	if (requested == corrupt_instruction)
		instruction->opcode = 0xff;
	return 1;
}

/* Logical 48-bit timer service, NOT the PIC Timer1 driver or its ISR races.
 * Keep the real field encoding and 32-bit delay/saturation contract. */
static uint64_t timer_ticks(const struct timer *timer)
{
	return ((uint64_t)timer->overflows << 16) | timer->timer1;
}

static void store_ticks(struct timer *timer, uint64_t value)
{
	CHECK(value < TICKS_NEVER);
	timer->timer1 = (unsigned short)(value & 0xffff);
	timer->overflows = (unsigned long)(value >> 16);
}

void timer_init(void) { ticks = 0; }
void timeoutnow(struct timer * const timer) { store_ticks(timer, ticks); }
void gettimestamp(struct timer * const timer) { timeoutnow(timer); }
void timeoutnever(struct timer * const timer)
{
	timer->timer1 = 0xffff;
	timer->overflows = 0xffffffffUL;
}
unsigned char timeoutneverexpires(const struct timer * const timer)
{
	return timer_ticks(timer) == TICKS_NEVER;
}
void settimeout(struct timer * const timer, unsigned long const duration)
{
	CHECK(duration <= UINT32_MAX);
	store_ticks(timer, ticks + duration);
}
void postponetimeout(struct timer * const timer, unsigned long const duration)
{
	CHECK(duration <= UINT32_MAX);
	store_ticks(timer, timer_ticks(timer) + duration);
}
unsigned char timeoutexpired(const struct timer * const timer)
{
	return !timeoutneverexpires(timer) && timer_ticks(timer) <= ticks;
}
unsigned long timestampdiff(const struct timer * const early,
			    const struct timer * const late)
{
	uint64_t first = timer_ticks(early), second = timer_ticks(late);
	uint64_t difference = first > second ? first - second : 0;

	return difference > UINT32_MAX ? UINT32_MAX : (unsigned long)difference;
}

static void drive_inputs(void)
{
	unsigned char input = BIT(STARTBUTTON_BIT) | BIT(SETUPBUTTON_BIT);

	if (buttons & START_BUTTON)
		input &= ~BIT(STARTBUTTON_BIT);
	if (buttons & SETUP_BUTTON)
		input &= ~BIT(SETUPBUTTON_BIT);
	if (hot)
		input |= HEATSENSOR_MASK;
	if (!water_high)
		input |= WATERVALVE_MASK;
	PORTB = (PORTB & ~TRISB) | (input & TRISB);
	PORTA = (PORTA & ~WATERSENSORANALOG_MASK) |
		(water_high ? WATERSENSORANALOG_MASK : 0);
}

void host_delay_ms(unsigned int duration)
{
	/* Startup settling only; the application timer is initialized afterwards. */
	(void)duration;
	drive_inputs();
}

static void account_time(uint64_t next)
{
	unsigned char active = actuators();
	unsigned int i;

	CHECK(next >= ticks);
	for (i = 0; i < COUNT(on_ticks); i++)
		if (active & BIT(i))
			on_ticks[i] += next - ticks;
	ticks = next;
}

static void schedule_peripherals(void)
{
#ifdef WATERSENSOR_ANALOG
	if (!ADCON0bits.GO)
		adc_due = TICKS_NEVER;
	else if (adc_due == TICKS_NEVER && !hold_adc)
		/* Supplied completion delay: 104us, NOT a verified hardware model. */
		adc_due = ticks + (SECOND * 104 + 999999) / 1000000;
	if (!TMR4IE || !T4CON)
		probe_due = TICKS_NEVER;
	else if (probe_due == TICKS_NEVER && !hold_probe) {
		CHECK(T4CON == 0x05 && PR4 == 124);
		/* Nominal 500us expiry, rounded up to the Timer1 tick. */
		probe_due = ticks + (SECOND + 1999) / 2000;
	}
#endif
}

static void advance_peripherals(uint64_t next)
{
#ifdef WATERSENSOR_ANALOG
	while (adc_due <= next || probe_due <= next) {
		if (adc_due <= probe_due) {
			account_time(adc_due);
			ADRES = adc_override >= 0 ? (unsigned int)adc_override :
				(water_high ? 900 : 22);
			ADCON0bits.GO = 0;
			adc_due = TICKS_NEVER;
		} else {
			account_time(probe_due);
			drive_inputs();
			if (noisy_comparator && water_quality() == WATER_QUALITY_GOOD) {
				/* Quality can pass, but normal level samples cannot qualify. */
				if (probes & 1)
					PORTB |= WATERVALVE_MASK;
				else
					PORTB &= ~WATERVALVE_MASK;
			}
			CHECK(GIE && TMR4IE);
			TMR4IF = 1;
			water_isr();
			CHECK(!TMR4IE && !T4CON && !TMR4IF);
			CHECK(!!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK) ==
				!!water_filling());
			probe_due = TICKS_NEVER;
			probes++;
			record("water ISR");
		}
	}
#endif
	account_time(next);
}

static void water_inputs(void)
{
	/* Scripted input response, not fluid dynamics or valve electronics. */
	if (!water_filling())
		fill_since = TICKS_NEVER;
	else if (fill_since == TICKS_NEVER)
		fill_since = ticks;
	else if (auto_fill && ticks - fill_since >= 2 * SECOND)
		water_high = 1;
	if (!get_Pump())
		drain_since = TICKS_NEVER;
	else if (drain_since == TICKS_NEVER)
		drain_since = ticks;
	else if (auto_drain && ticks - drain_since >= 2 * SECOND)
		water_high = 0;
	drive_inputs();
}

static void run_ms(unsigned long duration)
{
	while (duration--) {
		schedule_peripherals();
		advance_peripherals(ticks + MILISECOND);
		water_inputs();
		/* Match catgenius.c's cooperative worker order. Cat detection and
		 * serial input are inactive here; their hardware drivers are not linked. */
		rtc_work();
		record("RTC");
		water_work();
		record("water");
		catgenie_work();
		record("board");
		userinterface_work();
		record("UI");
		cmdline_work();
		record("command line");
		litterlanguage_work();
		record("interpreter");
		/* Check after all workers have had a chance to react this pass. */
		if (!litterlanguage_running() || litterlanguage_paused())
			CHECK(!actuators());
		if (get_Dryer())
			CHECK(water_valid() && !water_detected());
	}
}

static void boot(unsigned char box, unsigned char locked)
{
	unsigned char flags;

	memset(nvram, 0xff, sizeof(nvram));
	nvram[NVM_VERSION] = VERSION;
	nvram[NVM_MODE] = 0;
	nvram[NVM_KEYUNDLOCK] = !locked;
	nvram[NVM_BOXSTATE] = box;
	flags = catgenie_init();
	CHECK(flags == 0);
	timer_init();
	rtc_init(flags);
	water_init();
	userinterface_init(flags);
	cmdline_init();
	litterlanguage_init(flags);
	GIE = 1;
	record("boot");
}

static void gesture(unsigned char mask, unsigned long down_ms)
{
	buttons = mask;
	run_ms(down_ms);
	buttons = 0;
	run_ms(70);
}

static void start_wash(void)
{
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_running() && !litterlanguage_paused());
}

static void wait_actuator(unsigned char mask, unsigned long limit_ms)
{
	while (!(actuators() & mask) && limit_ms--)
		run_ms(1);
	CHECK(actuators() & mask);
}

static void finish_wash(void)
{
	unsigned int remaining = 20000;

	while (litterlanguage_running() && remaining--)
		run_ms(1);
	CHECK(!litterlanguage_running());
	run_ms(2); /* Let the UI consume completion/queued callbacks. */
	CHECK(!actuators() && !litterlanguage_paused());
	CHECK(nvram[NVM_BOXSTATE] == BOX_TIDY);
}

static void manual_wash(void)
{
	boot(BOX_TIDY, 0);
	start_wash();
#ifdef WATERSENSOR_ANALOG
	CHECK(!actuators() && nvram[NVM_BOXSTATE] == BOX_TIDY);
#endif
	wait_actuator(ACT_ARM, 4000);
	CHECK(get_Bowl() == BOWL_CCW && get_Arm() == ARM_DOWN);
	finish_wash();
	CHECK(seen_actuators == (ACT_WET | ACT_BOWL | ACT_ARM));
	CHECK(box_writes[BOX_MESSY] == 1 && box_writes[BOX_WET] == 1);
	CHECK(box_writes[BOX_TIDY] == 1 && !numbered_leds());
	CHECK(on_ticks[2] == 2 * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
#ifdef WATERSENSOR_ANALOG
	CHECK(probes > 0 && water_quality() == WATER_QUALITY_GOOD);
#endif
}

static void scoop_only(void)
{
	boot(BOX_TIDY, 0);
	gesture(START_BUTTON, 2070);
	CHECK(litterlanguage_running());
	finish_wash();
	CHECK(seen_actuators == (ACT_BOWL | ACT_ARM));
	CHECK(!box_writes[BOX_WET] && !on_ticks[2]);
}

static void pause_dosage(void)
{
	uint64_t delivered;

	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_DOSE, 8000);
	run_ms(300);
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_paused() && !actuators());
	delivered = on_ticks[2];
	run_ms(3000);
	CHECK(on_ticks[2] == delivered && !actuators());
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused() && get_Dosage());
	CHECK(get_Bowl() == BOWL_CCW);
	finish_wash();
	CHECK(on_ticks[2] == 2 * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
}

static void locked_buttons(void)
{
	boot(BOX_TIDY, 1);
	gesture(START_BUTTON, 70);
	gesture(START_BUTTON, 2070);
	gesture(SETUP_BUTTON, 70);
	CHECK(!litterlanguage_running() && !seen_actuators);
	CHECK(nvram[NVM_MODE] == 0 && !nvram[NVM_KEYUNDLOCK]);
	gesture(BUTTONS, 2070);
	CHECK(nvram[NVM_KEYUNDLOCK] == 1);
	start_wash();
	finish_wash();
}

static void simultaneous_start_overheat(void)
{
	boot(BOX_TIDY, 0);
	buttons = START_BUTTON;
	run_ms(70);
	buttons = 0;
	run_ms(50); /* Release will debounce on the next pass. */
	CHECK(!litterlanguage_running());
	hot = 1;
	run_ms(1);
	CHECK(!litterlanguage_running() && !litterlanguage_paused());
	CHECK(!seen_actuators && numbered_leds() == BIT(EVENT_ERR_OVERHEAT - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_TIDY);
	hot = 0;
	run_ms(1000);
	CHECK(!litterlanguage_running() && !numbered_leds());
	start_wash(); /* A new request is needed after cooling. */
	finish_wash();
}

static void overheat_during_dosage(void)
{
	uint64_t delivered;

	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_DOSE, 8000);
	run_ms(250);
	hot = 1;
	run_ms(1);
	CHECK(litterlanguage_paused() && !actuators());
	CHECK(numbered_leds() == BIT(EVENT_ERR_OVERHEAT - 1));
	delivered = on_ticks[2];
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_paused());
	hot = 0;
	run_ms(2500);
	CHECK(litterlanguage_paused() && !numbered_leds());
	CHECK(on_ticks[2] == delivered);
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused());
	finish_wash();
	CHECK(on_ticks[2] == 2 * SECOND * (DOSAGE_SECONDS_PER_ML / 10));
}

static void high_water_during_pause(void)
{
	auto_fill = 0;
	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_FILL, 4000);
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_paused() && !actuators());
	water_high = 1;
	run_ms(2500);
	CHECK(water_valid() && water_detected());
	CHECK(litterlanguage_paused() && !actuators());
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused() && !water_filling());
	CHECK(get_Dosage() && get_Bowl() == BOWL_CCW);
	finish_wash();
}

static void high_water_during_drying(void)
{
	unsigned int remaining = 2500;

	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_DRYER, 12000);
	water_high = 1;
	while (!litterlanguage_paused() && remaining--)
		run_ms(1);
	CHECK(litterlanguage_paused() && !actuators());
	run_ms(1); /* The UI consumes the interpreter's queued fault. */
	CHECK(numbered_leds() == BIT(EVENT_ERR_DRAINING - 1));
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_paused() && !actuators());
	water_high = 0;
	run_ms(2500);
	CHECK(water_valid() && !water_detected());
	CHECK(litterlanguage_paused());
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused() && get_Dryer());
	finish_wash();
	CHECK(!numbered_leds());
}

static void fill_timeout(void)
{
	auto_fill = 0;
	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_FILL, 4000);
	run_ms(MAX_FILLTIME / MILISECOND - 1);
	CHECK(!litterlanguage_paused() && water_filling());
	run_ms(1);
	CHECK(litterlanguage_paused() && !actuators());
	CHECK(on_ticks[0] == MAX_FILLTIME && !numbered_leds());
	run_ms(1);
	CHECK(numbered_leds() == BIT(EVENT_ERR_FILLING - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET);
	/* A long Start stops without falsely recording a completed wash. */
	gesture(START_BUTTON, 2070);
	CHECK(!litterlanguage_running() && !numbered_leds());
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET && !box_writes[BOX_TIDY]);
}

static void drain_timeout(void)
{
	auto_drain = 0;
	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_PUMP, 10000);
	run_ms(MAX_DRAINTIME / MILISECOND - 1);
	CHECK(!litterlanguage_paused() && get_Pump());
	/* WAITWATER starts the deadline one instruction after PUMP. */
	run_ms(2);
	CHECK(litterlanguage_paused() && !actuators());
	run_ms(1);
	CHECK(numbered_leds() == BIT(EVENT_ERR_DRAINING - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET);
	water_high = 0;
	run_ms(2500);
	CHECK(water_valid() && !water_detected() && litterlanguage_paused());
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused());
	finish_wash();
	CHECK(!numbered_leds());
}

static void corrupt_recipe(void)
{
	corrupt_instruction = &washprogram[8]; /* Replace WAITDOSAGE with an invalid opcode. */
	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_DOSE, 8000);
	run_ms(1);
	/* Execution reports the fault after the UI has already run this pass. */
	CHECK(litterlanguage_running() && get_Dosage() && !numbered_leds());
	run_ms(1);
	CHECK(!litterlanguage_running() && !actuators());
	CHECK(numbered_leds() == BIT(EVENT_ERR_EXECUTION - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET && !box_writes[BOX_TIDY]);
	run_ms(1000);
	CHECK(!litterlanguage_running());
}

static void wet_boot_cleanup(void)
{
	water_high = 1;
	boot(BOX_WET, 0);
	CHECK(litterlanguage_running());
	finish_wash();
	CHECK(seen_actuators == ACT_PUMP && !box_writes[BOX_MESSY]);
	CHECK(water_valid() && !water_detected());
}

#ifdef WATERSENSOR_ANALOG
static void check_failed_preflight(void)
{
	CHECK(!litterlanguage_running() && !litterlanguage_paused());
	CHECK(!seen_actuators && numbered_leds() == BIT(EVENT_ERR_EXECUTION - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_TIDY);
	CHECK(!box_writes[BOX_MESSY] && !box_writes[BOX_WET]);
}

static void paused_preflight(void)
{
	boot(BOX_TIDY, 0);
	start_wash();
	gesture(START_BUTTON, 70);
	CHECK(litterlanguage_paused() && !seen_actuators);
	run_ms(2000);
	CHECK(water_quality() == WATER_QUALITY_GOOD && water_valid());
	CHECK(litterlanguage_paused() && nvram[NVM_BOXSTATE] == BOX_TIDY);
	gesture(START_BUTTON, 70);
	CHECK(!litterlanguage_paused());
	finish_wash();
}

static void optical_preflight_fault(unsigned char pause)
{
	adc_override = 800;
	boot(BOX_TIDY, 0);
	start_wash();
	if (pause) {
		gesture(START_BUTTON, 70);
		CHECK(litterlanguage_paused());
	}
	run_ms(1000);
	check_failed_preflight();
	CHECK(water_quality() == WATER_QUALITY_OPTICAL && !water_failed());
	adc_override = 22;
	run_ms(1500);
	CHECK(water_quality() == WATER_QUALITY_GOOD && water_valid());
	check_failed_preflight(); /* Clearing the sensor never restarts the request. */
	start_wash();
	finish_wash();
	CHECK(!numbered_leds());
}

static void optical_preflight(void) { optical_preflight_fault(0); }
static void optical_paused_preflight(void) { optical_preflight_fault(1); }

static void level_preflight(void)
{
	water_high = 1;
	boot(BOX_TIDY, 0);
	start_wash();
	run_ms(1000);
	check_failed_preflight();
	CHECK(water_quality() == WATER_QUALITY_LEVEL && !water_failed());
	water_high = 0;
	run_ms(1500);
	CHECK(water_quality() == WATER_QUALITY_LEVEL);
	check_failed_preflight();
	start_wash(); /* Level faults require a new check, not passive recovery. */
	finish_wash();
}

static void unqualified_preflight(void)
{
	noisy_comparator = 1;
	adc_override = 22;
	boot(BOX_TIDY, 0);
	start_wash();
	run_ms(4900);
	CHECK(litterlanguage_running() && !water_valid());
	CHECK(water_quality() == WATER_QUALITY_GOOD && !seen_actuators);
	run_ms(100);
	check_failed_preflight();
	CHECK(!water_failed());
	noisy_comparator = 0;
	run_ms(1000);
	CHECK(water_valid() && !litterlanguage_running());
}

static void acquisition_preflight_fault(unsigned char probe)
{
	boot(BOX_TIDY, 0);
	start_wash();
	hold_probe = probe;
	hold_adc = !probe;
	run_ms(200);
	check_failed_preflight();
	CHECK(water_failed() && !water_valid());
	CHECK(water_acquisition_fault() == (probe ?
		WATER_ACQUISITION_PROBE : WATER_ACQUISITION_ADC));
	hold_probe = hold_adc = 0;
	run_ms(1500);
	CHECK(!water_failed() && water_valid());
	check_failed_preflight();
	start_wash();
	finish_wash();
}

static void adc_preflight(void) { acquisition_preflight_fault(0); }
static void probe_preflight(void) { acquisition_preflight_fault(1); }

static void acquisition_program_fault(unsigned char probe, unsigned char pause)
{
	auto_fill = 0;
	boot(BOX_TIDY, 0);
	start_wash();
	wait_actuator(ACT_FILL, 4000);
	if (pause) {
		gesture(START_BUTTON, 70);
		CHECK(litterlanguage_paused());
	}
	hold_probe = probe;
	hold_adc = !probe;
	run_ms(200);
	CHECK(water_failed() && !water_valid());
	CHECK(water_acquisition_fault() == (probe ?
		WATER_ACQUISITION_PROBE : WATER_ACQUISITION_ADC));
	CHECK(!litterlanguage_running() && !litterlanguage_paused() && !actuators());
	CHECK(numbered_leds() == BIT(EVENT_ERR_EXECUTION - 1));
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET && !box_writes[BOX_TIDY]);
	hold_probe = hold_adc = 0;
	run_ms(1500);
	CHECK(!water_failed() && water_valid() && !litterlanguage_running());
	CHECK(nvram[NVM_BOXSTATE] == BOX_WET);
	auto_fill = 1;
	start_wash();
	finish_wash();
}

static void adc_active(void) { acquisition_program_fault(0, 0); }
static void adc_paused(void) { acquisition_program_fault(0, 1); }
static void probe_active(void) { acquisition_program_fault(1, 0); }
static void probe_paused(void) { acquisition_program_fault(1, 1); }
#endif

static const struct scenario {
	const char *name;
	void (*run)(void);
} scenarios[] = {
	{"manual-wash", manual_wash},
	{"scoop-only", scoop_only},
	{"pause-dosage", pause_dosage},
	{"locked-buttons", locked_buttons},
	{"simultaneous-start-overheat", simultaneous_start_overheat},
	{"overheat-during-dosage", overheat_during_dosage},
	{"high-water-during-pause", high_water_during_pause},
	{"high-water-during-drying", high_water_during_drying},
	{"fill-timeout", fill_timeout},
	{"drain-timeout", drain_timeout},
	{"corrupt-recipe", corrupt_recipe},
	{"wet-boot-cleanup", wet_boot_cleanup},
#ifdef WATERSENSOR_ANALOG
	{"paused-preflight", paused_preflight},
	{"optical-preflight", optical_preflight},
	{"optical-paused-preflight", optical_paused_preflight},
	{"level-preflight", level_preflight},
	{"unqualified-preflight", unqualified_preflight},
	{"adc-preflight", adc_preflight},
	{"probe-preflight", probe_preflight},
	{"adc-active", adc_active},
	{"adc-paused", adc_paused},
	{"probe-active", probe_active},
	{"probe-paused", probe_paused},
#endif
};

int main(int argc, char **argv)
{
	size_t i;

	/* A separate invocation for each scenario gives genuine C startup state:
	 * no private-state pokes or test reset hooks in production modules. */
	if (argc == 2 && !strcmp(argv[1], "--list")) {
		for (i = 0; i < COUNT(scenarios); i++)
			puts(scenarios[i].name);
		return EXIT_SUCCESS;
	}
	for (i = 0; argc == 2 && i < COUNT(scenarios); i++)
		if (!strcmp(argv[1], scenarios[i].name)) {
			scenario_name = scenarios[i].name;
			scenarios[i].run();
			printf("Integration: %s passed\n", scenario_name);
			return EXIT_SUCCESS;
		}
	fprintf(stderr, "Usage: %s --list|SCENARIO\n", argv[0]);
	return EXIT_FAILURE;
}
