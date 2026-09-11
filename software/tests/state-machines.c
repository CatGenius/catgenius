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
const struct instruction washprogram[] = {{INS_END, 0}};
const struct instruction cleanupprogram[] = {{INS_END, 0}};
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
	timeoutnever(&timer_waitins);
	timeoutnever(&timer_fill);
	timeoutnever(&timer_drain);
	timeoutnever(&timer_autodose);
	state = LED_ON;
	hysteresis = 0;
	filling = detected = ledalwayson = 0;
	timeoutnow(&sensortimer);
	ADCON0bits.GO = 0;
	water_init();
}

static void sample_water(unsigned int value)
{
	assert(state == LED_ON);
	ticks = sensortimer.overflows;
	water_work();
	ticks = sensortimer.overflows;
#ifdef WATERSENSOR_ANALOG
	water_work();
	assert(ADCON0bits.GO && state == PROCESS_RESULT);
	ADRES = value;
	ADCON0bits.GO = 0;
#else
	if (value > UNDETECTION_THRESHOLD - DETECTION_MARGIN)
		PORTA |= WATERSENSORANALOG_MASK;
	else
		PORTA &= ~WATERSENSORANALOG_MASK;
#endif
	water_work();
	assert(state == LED_ON);
}

static void qualify_water(unsigned int value)
{
	unsigned char i;

	for (i = 0; i < HYSTERESIS_MAX; i++)
		sample_water(value);
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

int main(void)
{
	test_water_sampling();
	test_stop_outputs();
	test_dry_program();
	test_buttons();
	test_heat_fault();
	test_pause_context();
	puts("Host state-machine checks passed.");
	return 0;
}
