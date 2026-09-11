/* Execute timer.c with 16/32-bit PIC arithmetic and six-byte timer storage. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <xc.h>
#include "../common/hardware.h"

static unsigned char TMR1CS, TMR1CS0, TMR1CS1;
static unsigned char T1CKPS0, T1CKPS1, T1OSCEN, nT1SYNC, TMR1ON, TMR1IE;
static unsigned char TMR1IF;
static uint16_t hardware_ticks;
static unsigned char advance_on_low_read;
static unsigned char read_low(void);
#define TMR1H ((unsigned char)(hardware_ticks >> 8))
#define TMR1L read_low()

/* Only the included timer module sees this host-only layout adaptation.
 * Standard headers were included above, outside the macro/packing scope. */
#define long int
#pragma pack(push, 1)
#include "../common/timer.c"
#pragma pack(pop)
#undef long

static unsigned char read_low(void)
{
	if (advance_on_low_read) {
		advance_on_low_read = 0;
		hardware_ticks++;
		if (!hardware_ticks) {
			TMR1IF = 1;
			timer_isr();
			TMR1IF = 0;
		}
	}
	return (unsigned char)hardware_ticks;
}

static uint64_t unpack(const struct timer *value)
{
	return ((uint64_t)value->overflows << 16) | value->timer1;
}

static void clock_at(uint64_t value)
{
	overflows = (uint32_t)(value >> 16);
	hardware_ticks = (uint16_t)value;
	TMR1IF = advance_on_low_read = 0;
}

static void test_arithmetic(void)
{
	static const uint64_t times[] = {0, 1, 0xffff, 0xfffffffe,
		0xffffffff, 0x100000000ULL, 0x12345678abcdULL};
	static const uint32_t delays[] = {0, 1, 0xffff, 0x10000, 0xfffffffe, 0xffffffff};
	unsigned int i, j;
	struct timer deadline, now;
	assert(sizeof(unsigned short) == 2 && sizeof(unsigned int) == 4);
	assert(sizeof(struct timer) == 6 && sizeof(struct longshort) == 6);
	for (i = 0; i < sizeof(times) / sizeof(times[0]); i++) {
		clock_at(times[i]);
		gettimestamp(&now);
		assert(unpack(&now) == times[i]);
		for (j = 0; j < sizeof(delays) / sizeof(delays[0]); j++) {
			settimeout(&deadline, delays[j]);
			assert(unpack(&deadline) == times[i] + delays[j]);
			assert(timestampdiff(&deadline, &now) == delays[j]);
			assert(timeoutexpired(&deadline) == (delays[j] == 0));
			postponetimeout(&deadline, 1);
			assert(unpack(&deadline) == times[i] + (uint64_t)delays[j] + 1);
			assert(timestampdiff(&deadline, &now) ==
			       (delays[j] == UINT32_MAX ? UINT32_MAX : delays[j] + 1));
		}
		timeoutnever(&deadline);
		assert(timeoutneverexpires(&deadline) && !timeoutexpired(&deadline));
		assert(timestampdiff(&deadline, &now) == UINT32_MAX);
		timeoutnow(&deadline);
		assert(timeoutexpired(&deadline) && !timeoutneverexpires(&deadline));
		assert(timestampdiff(&deadline, &now) == 0);
	}
}

static void test_read_rollovers(void)
{
	struct timer now;
	clock_at(0x12ff);
	advance_on_low_read = 1;
	gettimestamp(&now);
	assert(unpack(&now) == 0x1300);
	clock_at(0x1234ffff);
	advance_on_low_read = 1;
	gettimestamp(&now);
	assert(unpack(&now) == 0x12350000);
}

int main(void)
{
	test_arithmetic();
	test_read_rollovers();
	puts("Host timer checks passed.");
	return 0;
}
