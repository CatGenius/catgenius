#include <htc.h>

#include "timer.h"
#include "catgenie120.h"

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);

static unsigned char		test		= 0;


void init (void)
{
	unsigned char temp;

	/* Init the hardware */
	init_catgenie();

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
			set_LED_1(test & 1);
			set_LED_2(test & 1);
			set_LED_3(test & 1);
			set_LED_4(test & 1);
			set_LED_Cat(test & 1);
			set_LED_Error(test & 1);
			set_LED_Cartridge(test & 1);
//			set_Beeper(test & 1);
		}
		if (PORTB & 0x20)
			set_LED_Locked(test & 1);
	}
}