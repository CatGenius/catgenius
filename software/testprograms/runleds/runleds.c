#include <htc.h>

#include "timer.h"

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);

static unsigned char		test		= 0;


void init (void)
{
	unsigned char temp;

	TRISC = 0xFE;		/* Port C all outputs */

	/* initialize timer 1 */
	timer_init();

	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

void main (void)
{
	unsigned long	delay = SECOND/5;
	struct timer	test_timer ;

	/* Initialize the hardware */
	init();

	settimeout(&test_timer, delay);

	/* Execute the run loop */
	for(;;){
		if (timeoutexpired(&test_timer))
		{
			settimeout(&test_timer, delay);
			test++;
			PORTC = test & 0x01;
		}

	}
}