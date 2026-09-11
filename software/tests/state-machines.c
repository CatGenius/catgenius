/* Run the actual firmware C with supplied GPIO, ADC and timer inputs. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../common/hardware.h"
/* PIC ROM addresses and their debug formatting are not host pointers. */
#undef LL_DEBUG

static char output[8192];
static int firmware_printf(const char *format, ...)
{
	va_list args;
	size_t used = strlen(output);
	int result;

	va_start(args, format);
	result = vsnprintf(output + used, sizeof(output) - used, format, args);
	va_end(args);
	assert(result >= 0 && (size_t)result < sizeof(output) - used);
	return result;
}

#define printf firmware_printf
#include "../common/catgenie120.c"
#include "../common/water.c"
#include "../catgenius/litterlanguage.c"
#include "../common/cmdline_box.c"
#undef printf

volatile unsigned char PORTA, PORTB, PORTC, PORTD, PORTE;
volatile unsigned char LATA, LATB, LATC, LATD, LATE;
volatile unsigned char TRISA, TRISB, TRISC, TRISD, TRISE;
volatile unsigned char ANSELA, ANSELB, ANSELD, ANSELE;
volatile unsigned char WPUB, WPUE, nWPUEN, nRBPU;
volatile unsigned char RBIF, RBIE, IOCBP, IOCBN, IOCBF, IOCIF, IOCIE;
volatile unsigned char ADCON1;
volatile unsigned int ADRES;
volatile unsigned char TMR4, PR4, T4CON, TMR4IE, TMR4IF, GIE;
volatile struct host_adcon0 ADCON0bits;
volatile struct host_adcon1 ADCON1bits;
bit cat_detected, overheated;

static unsigned long ticks;
static unsigned char nvram[256];
static unsigned char input_b;
static unsigned int button_events[2][2];
static unsigned int events[6][2];
static struct instruction program[] = {
	{INS_WAITTIME, 10}, {INS_DRYER, 1}, {INS_DRYER, 0}, {INS_END, 0}
};
const struct instruction washprogram[] = {
	{INS_START, FLAGS_WETRUN | FLAGS_DRYRUN | INS_END},
	{INS_PUMP, 1}, {INS_WAITTIME, 500}, {INS_END, 0}
};
const struct instruction cleanupprogram[] = {
	{INS_START, FLAGS_WETRUN | FLAGS_DRYRUN | INS_END},
	{INS_PUMP, 1}, {INS_END, 0}
};
static struct instruction const *requested;

unsigned char eeprom_read(unsigned char address) { return nvram[address]; }
void eeprom_write(unsigned char address, unsigned char value) { nvram[address] = value; }
void printtime(void) {}
void host_delay_ms(unsigned int duration)
{
	(void)duration;
	PORTB = (PORTB & ~TRISB) | (input_b & TRISB);
}
void startbutton_event(unsigned char up)
{
	assert(up <= 1);
	button_events[0][up]++;
}
void setupbutton_event(unsigned char up)
{
	assert(up <= 1);
	button_events[1][up]++;
}
void litterlanguage_event(unsigned char event, unsigned char argument)
{
	assert(event < 6 && argument <= 1);
	events[event][argument]++;
}
void romwashprogram_reqins(struct instruction const *address) { requested = address; }
unsigned char romwashprogram_getins(struct instruction * const instruction)
{
	*instruction = *requested;
	return 1;
}
void timeoutnow(struct timer * const timer_p)
{
	timer_p->timer1 = 0;
	timer_p->overflows = ticks;
}
void timeoutnever(struct timer * const timer_p)
{
	timer_p->timer1 = 0xffff;
	timer_p->overflows = 0xffffffffUL;
}
unsigned char timeoutneverexpires(struct timer const * const timer_p)
{
	return timer_p->timer1 == 0xffff && timer_p->overflows == 0xffffffffUL;
}
void settimeout(struct timer * const timer_p, unsigned long const duration)
{
	timer_p->timer1 = 0;
	timer_p->overflows = ticks + duration;
}
unsigned char timeoutexpired(struct timer const * const timer_p)
{
	return !timeoutneverexpires(timer_p) && timer_p->overflows <= ticks;
}
void gettimestamp(struct timer * const timer_p) { timeoutnow(timer_p); }
unsigned long timestampdiff(struct timer const * const early_p,
			    struct timer const * const late_p)
{
	if (timeoutneverexpires(early_p))
		return 0xffffffffUL;
	return early_p->overflows > late_p->overflows ?
		early_p->overflows - late_p->overflows : 0;
}

static void reset_firmware(void)
{
	unsigned int i;

	ticks = 0;
	output[0] = 0;
	memset(nvram, 0, sizeof(nvram));
	memset(button_events, 0, sizeof(button_events));
	memset(events, 0, sizeof(events));
	input_b = BIT(STARTBUTTON_BIT) | BIT(SETUPBUTTON_BIT);
	heat_old = 0;
	beep_bits = 0;
	LATA = LATB = LATC = LATD = LATE = 0;
	assert(catgenie_init() == 0);
	for (i = 0; i < DEBOUNCER_MAX; i++)
		timeoutnever(&debouncers[i].timer);
	for (i = 0; i < PACER_MAX; i++) {
		timeoutnever(&pacers[i].timer);
		pacers[i].pattern = 0;
	}
	ins_state = STATE_IDLE;
	ins_pointer = program;
	wet_program = paused = 0;
	error_fill = error_drain = error_overheat = error_flood = error_execution = 0;
#ifdef WATERSENSOR_ANALOG
	check_before_program = check_started = 0;
#endif
	timeoutnever(&timer_waitins);
	timeoutnever(&timer_fill);
	timeoutnever(&timer_drain);
	timeoutnever(&timer_autodose);
#ifndef WATERSENSOR_ANALOG
	state = LED_ON;
	hysteresis = samples = 0;
	reflectionquality = 0;
	valid = failed = filling = detected = ledalwayson = 0;
	timeoutnow(&sensortimer);
#endif
	ADCON0bits.GO = 0;
	GIE = 1;
	water_init();
}

#ifdef WATERSENSOR_ANALOG
static void start_analog_cycle(void)
{
	assert(state == LED_ON);
	if (ticks < polltimer.overflows)
		ticks = polltimer.overflows;
	water_work();
	assert(state == START_CONVERSION);
}

static void begin_analog_conversion(void)
{
	start_analog_cycle();
	ticks = sensortimer.overflows;
	water_work();
	assert(ADCON0bits.GO && state == PROCESS_RESULT);
}

static void complete_probe(unsigned char high)
{
	assert(state == WAIT_PROBE && TMR4IE && T4CON == 0x05);
	assert(PR4 == PROBE_COUNTS - 1 && GIE);
	assert(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK);
	assert(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK);
	if (high)
		PORTB |= WATERVALVE_MASK;
	else
		PORTB &= ~WATERVALVE_MASK;
	ticks += (SECOND + 1999) / 2000;
	TMR4IF = 1;
	water_isr();
	assert(!TMR4IE && !TMR4IF && !T4CON && probe_done);
	assert(!!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK) == !!water_filling());
}

static void sample_analog(unsigned int first, unsigned int second,
			  unsigned int third, unsigned int fourth,
			  unsigned char comparator_high)
{
	unsigned char i;
	unsigned int values[] = {first, second, third, fourth};

	start_analog_cycle();
	for (i = 0; i < 4; i++) {
		ticks = sensortimer.overflows;
		water_work();
		assert(ADCON0bits.GO && state == PROCESS_RESULT);
		ADRES = values[i];
		ADCON0bits.GO = 0;
		water_work();
	}
	if (state == WAIT_PROBE) {
		complete_probe(comparator_high);
		water_work();
	}
	assert(state == LED_ON);
}
#endif

static void sample_water(unsigned int value)
{
#ifdef WATERSENSOR_ANALOG
	sample_analog(value, value, value, value,
		      value <= UNDETECTION_THRESHOLD - DETECTION_MARGIN);
#else
	assert(state == LED_ON);
	ticks = sensortimer.overflows;
	water_work();
	ticks = sensortimer.overflows;
	if (value > UNDETECTION_THRESHOLD - DETECTION_MARGIN)
		PORTA |= WATERSENSORANALOG_MASK;
	else
		PORTA &= ~WATERSENSORANALOG_MASK;
	water_work();
	assert(state == LED_ON);
#endif
}

static void qualify_water(unsigned int value)
{
	unsigned char i;

	for (i = 0; i < HYSTERESIS_MAX; i++)
		sample_water(value);
	assert(water_valid() && !water_failed());
}

static void instruction(unsigned char opcode, unsigned int operand)
{
	wet_program = 1;
	ins_state = STATE_GET_INS;
	cur_instruction.opcode = opcode;
	cur_instruction.operant = operand;
	program[0] = cur_instruction;
	exe_instruction();
}

static void assert_stopped_outputs(void)
{
	assert(!get_Pump() && !get_Dosage() && !get_Dryer() && !water_filling());
	assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
	assert(get_Bowl() == BOWL_STOP && get_Arm() == ARM_STOP);
}

static void test_water_sampling(void)
{
	reset_firmware();
	assert(!water_detected());
	qualify_water(600);
	assert(water_detected());
	qualify_water(UNDETECTION_THRESHOLD - DETECTION_MARGIN);
	assert(!water_detected());
	assert(!(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK));
	water_fill(1);
	sample_water(0);
	assert(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK);
	assert(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK);
	water_fill(0);
	sample_water(0);
	assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
	assert(!(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK));
}

static void test_stop_outputs(void)
{
	reset_firmware();
	instruction(INS_WAITTIME, 1000);
	set_Bowl(BOWL_CW);
	set_Arm(ARM_DOWN);
	water_fill(1);
	set_Pump(1);
	set_Dosage(1);
	set_Dryer(1);
	litterlanguage_stop();
	assert(ins_state == STATE_IDLE);
	assert_stopped_outputs();
}

static void test_dry_program(void)
{
	reset_firmware();
	ins_state = STATE_GET_INS;
	cur_instruction.opcode = INS_WAITWATER;
	cur_instruction.operant = 1;
	exe_instruction();
	assert(ins_pointer == program + 1 && ins_state == STATE_FETCH_INS);
	assert(timeoutneverexpires(&timer_fill));
	cur_instruction.opcode = INS_DRYER;
	exe_instruction();
	assert(ins_pointer == program + 2 && !get_Dryer() && !paused);
}

static void test_buttons(void)
{
	unsigned char button, cycle;
	unsigned char masks[] = {BIT(STARTBUTTON_BIT), BIT(SETUPBUTTON_BIT)};

	reset_firmware();
	assert(debouncers[0].state == 1 && debouncers[1].state == 1);
	for (button = 0; button < 2; button++) {
		for (cycle = 0; cycle < 2; cycle++) {
			PORTB &= ~masks[button];
			catgenie_work();
			ticks += BUTTON_DEBOUNCE - 1;
			catgenie_work();
			assert(button_events[button][0] == cycle);
			ticks++;
			catgenie_work();
			assert(button_events[button][0] == cycle + 1U);
			PORTB |= masks[button];
			catgenie_work();
			ticks += BUTTON_DEBOUNCE;
			catgenie_work();
			assert(button_events[button][1] == cycle + 1U);
		}
	}
}

static void test_heat_fault(void)
{
	unsigned char cycle;

	reset_firmware();
	PORTB |= HEATSENSOR_MASK;
	for (cycle = 0; cycle < 10; cycle++)
		catgenie_work();
	assert(events[EVENT_ERR_OVERHEAT][1] == 1);
	PORTB &= ~HEATSENSOR_MASK;
	for (cycle = 0; cycle < 10; cycle++)
		catgenie_work();
	assert(events[EVENT_ERR_OVERHEAT][0] == 1);
	qualify_water(0);
	instruction(INS_WAITTIME, 1000);
	PORTB |= HEATSENSOR_MASK;
	catgenie_work();
	litterlanguage_work();
	assert(paused && error_overheat);
	assert_stopped_outputs();
	litterlanguage_pause(0);
	assert(paused);
}

static void test_pause_context(void)
{
	unsigned char combination, direction;
	unsigned long before;

	for (direction = 1; direction <= 2; direction++)
		for (combination = 0; combination < 16; combination++) {
			reset_firmware();
			qualify_water(0);
			instruction(INS_WAITTIME, 1000);
			set_Bowl(direction);
			set_Arm(direction);
			water_fill(combination & 1);
			set_Pump(combination & 2);
			set_Dosage(combination & 4);
			set_Dryer(combination & 8);
			settimeout(&timer_fill, 2 * SECOND);
			settimeout(&timer_drain, 3 * SECOND);
			settimeout(&timer_autodose, 4 * SECOND);
			before = ticks;
			litterlanguage_pause(0xff);
			assert(litterlanguage_paused());
			assert_stopped_outputs();
			assert(timeoutneverexpires(&timer_waitins));
			litterlanguage_pause(2);
			ticks += 5 * SECOND;
			litterlanguage_pause(0);
			assert(!litterlanguage_paused());
			assert(get_Bowl() == direction && get_Arm() == direction);
			assert(water_filling() == !!(combination & 1));
			assert(get_Pump() == ((combination & 2) ? PUMP_MASK : 0));
			assert(get_Dosage() == ((combination & 4) ? DOSAGE_MASK : 0));
			assert(get_Dryer() == ((combination & 8) ? DRYER_MASK : 0));
			assert(timer_waitins.overflows == ticks + SECOND);
			assert(timer_fill.overflows == ticks + 2 * SECOND);
			assert(timer_drain.overflows == ticks + 3 * SECOND);
			assert(timer_autodose.overflows == ticks + 4 * SECOND);
			assert(ticks > before);
		}
}

static void test_fill_resume(void)
{
	reset_firmware();
	qualify_water(0);
	instruction(INS_WAITTIME, 1000);
	water_fill(1);
	litterlanguage_pause(1);
	qualify_water(600);
	litterlanguage_pause(0);
	assert(!paused && water_detected() && !water_filling());
	assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
}

static void test_water_waits(void)
{
	unsigned long deadline;

	reset_firmware();
	qualify_water(600);
	instruction(INS_WAITWATER, 0);
	assert(!get_Pump());
	ticks += MAX_DRAINTIME - 1;
	litterlanguage_work();
	assert(!paused && ins_pointer == program);
	ticks++;
	litterlanguage_work();
	assert(paused && error_drain && ins_pointer == program);
	assert_stopped_outputs();
	assert(events[EVENT_ERR_DRAINING][1] == 1);
	litterlanguage_work();
	assert(events[EVENT_ERR_DRAINING][1] == 1);
	litterlanguage_pause(0);
	assert(timer_drain.overflows == ticks + MAX_DRAINTIME);
	/* Keep the fake sensor schedule in step with the advanced clock. */
	timeoutnow(&sensortimer);
	qualify_water(0);
	litterlanguage_work();
	assert(ins_pointer == program + 1 && timeoutneverexpires(&timer_drain));

	reset_firmware();
	qualify_water(0);
	instruction(INS_WAITWATER, 1);
	assert(!water_filling());
	ticks += MAX_FILLTIME;
	litterlanguage_work();
	assert(paused && error_fill && ins_pointer == program);
	assert(events[EVENT_ERR_FILLING][1] == 1);

	reset_firmware();
	qualify_water(0);
	settimeout(&timer_fill, SECOND);
	deadline = timer_fill.overflows;
	instruction(INS_WAITWATER, 1);
	assert(timer_fill.overflows == deadline);

}

static void test_water_qualification(void)
{
	unsigned char i;

	reset_firmware();
	assert(!water_valid());
	for (i = 1; i < HYSTERESIS_MAX; i++) {
		sample_water(0);
		assert(!water_valid());
	}
	sample_water(0);
	assert(water_valid() && !water_detected());
	for (i = 1; i < HYSTERESIS_MAX; i++) {
		sample_water(600);
		assert(!water_detected());
	}
	sample_water(600);
	assert(water_detected());
#ifdef WATERSENSOR_ANALOG
	assert(water_reflectionquality() == 600);
#else
	assert(water_reflectionquality() == DETECTION_THRESHOLD);
#endif
	qualify_water(UNDETECTION_THRESHOLD - DETECTION_MARGIN);
	assert(!water_detected());
	water_ledalwayson(2);
	sample_water(0);
	assert(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK);
	water_ledalwayson(0);
	sample_water(0);
	assert(!(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK));
}

static void test_unqualified_wait(void)
{
	reset_firmware();
	instruction(INS_WAITWATER, 0);
	litterlanguage_work();
	assert(!water_valid() && ins_pointer == program);
	ticks += MAX_DRAINTIME;
	litterlanguage_work();
	assert(paused && error_drain);
}

#ifdef WATERSENSOR_ANALOG
static void fail_conversion(void)
{
	begin_analog_conversion();
	assert(ADCON0bits.GO);
	ADRES = 1023;
	ticks = sensortimer.overflows;
	water_work();
	assert(!ADCON0bits.GO && water_failed() && !water_valid());
}

static void test_adc_timeout(void)
{
	unsigned char i, was_paused;

	reset_firmware();
	qualify_water(0);
	water_fill(1);
	begin_analog_conversion();
	ADRES = 1023;
	ticks = sensortimer.overflows - 1;
	water_work();
	assert(ADCON0bits.GO && water_valid() && !water_failed());
	assert(water_filling() && water_reflectionquality() == 0);
	ticks++;
	water_work();
	assert(!ADCON0bits.GO && water_failed() && !water_valid());
	assert(!water_filling() && !water_detected());
	assert(water_reflectionquality() == 0);
	assert(!(WATERVALVEPULLUP(LAT) & WATERVALVEPULLUP_MASK));
	assert(!(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK));
	water_fill(1);
	assert(!water_filling());
	for (i = 1; i < HYSTERESIS_MAX; i++) {
		sample_water(0);
		assert(!water_valid() && water_failed());
	}
	sample_water(0);
	assert(water_valid() && !water_failed() && !water_filling());
	water_fill(1);
	assert(water_filling());
	water_ledalwayson(1);
	fail_conversion();
	assert(WATERSENSOR_LED(LAT) & WATERSENSOR_LED_MASK);
	assert(!water_filling());

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
		set_Dryer(!was_paused);
		if (was_paused)
			litterlanguage_pause(1);
		fail_conversion();
		if (was_paused) {
			litterlanguage_pause(0);
			assert(paused);
			assert_stopped_outputs();
		}
		litterlanguage_work();
		assert(ins_state == STATE_IDLE && !paused && error_execution);
		assert_stopped_outputs();
		assert(events[EVENT_ERR_EXECUTION][1] == 1);
		assert(nvram[NVM_BOXSTATE] == BOX_WET);
		qualify_water(0);
		litterlanguage_work();
		assert(ins_state == STATE_IDLE);
	}
}
#endif

static void test_dryer_interlock(void)
{
	reset_firmware();
	instruction(INS_DRYER, 1);
	assert(paused && error_drain && !get_Dryer() && ins_pointer == program);

	reset_firmware();
	qualify_water(600);
	instruction(INS_DRYER, 1);
	assert(paused && error_drain && !get_Dryer() && ins_pointer == program);
	qualify_water(0);
	litterlanguage_pause(0);
	litterlanguage_work();
	/* The blocked dryer instruction is fetched again on retry. */
	assert(ins_pointer == program + 1 && get_Dryer());

	reset_firmware();
	qualify_water(0);
	instruction(INS_DRYER, 1);
	assert(get_Dryer() && !paused && ins_pointer == program + 1);
	qualify_water(600);
	litterlanguage_work();
	assert(paused && error_drain);
	assert_stopped_outputs();
	litterlanguage_pause(0);
	assert(paused && !get_Dryer());
	qualify_water(0);
	litterlanguage_pause(0);
	assert(!paused && get_Dryer());

	reset_firmware();
	qualify_water(600);
	set_Dryer(1);
	instruction(INS_DRYER, 0);
	assert(!get_Dryer() && !paused);
}

static void test_diagnostics(void)
{
	char *args[] = {"water"};

	reset_firmware();
	assert(water(1, args) == ERR_OK);
	assert(strstr(output, "Water: unqualified"));
	assert(strstr(output, "(fill inhibited)"));
	qualify_water(0);
	output[0] = 0;
	assert(water(1, args) == ERR_OK);
	assert(strstr(output, "Water: low"));
#ifdef WATERSENSOR_ANALOG
	assert(strstr(output, "Reflection ADC"));
#else
	assert(strstr(output, "Reflection digital"));
#endif
	qualify_water(600);
	output[0] = 0;
	assert(water(1, args) == ERR_OK);
	assert(strstr(output, "Water: high"));
#ifdef WATERSENSOR_ANALOG
	fail_conversion();
	output[0] = 0;
	assert(water(1, args) == ERR_OK);
	assert(strstr(output, "Water: sensor timeout"));
	assert(strstr(output, "Reflection ADC (last completed): 600"));
#endif
	assert(water(2, args) == ERR_SYNTAX);
}

#include "water-protocol.h"
#include "water-preflight.h"

int main(void)
{
	test_water_sampling();
	test_stop_outputs();
	test_dry_program();
	test_buttons();
	test_heat_fault();
	test_pause_context();
	test_fill_resume();
	test_water_waits();
	test_water_qualification();
	test_unqualified_wait();
#ifdef WATERSENSOR_ANALOG
	test_adc_timeout();
#endif
	test_dryer_interlock();
	test_diagnostics();
	test_program_start_compatibility();
#ifdef WATERSENSOR_ANALOG
	test_probe_lifecycle();
	test_analog_level_and_mean();
	test_quality_acquisition();
	test_preflight_success_and_timeout();
	test_preflight_quality_faults();
	test_preflight_acquisition_faults();
#endif
	puts("Host state-machine checks passed.");
	return 0;
}
