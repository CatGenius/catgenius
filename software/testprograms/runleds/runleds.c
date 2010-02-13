#include <htc.h>

#include "timer.h"
#include "catgenie120.h"

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);


static unsigned char	PORTB_old;
static unsigned char	test		= 0;


void init (void)
{
	unsigned char temp;

	/* Init the hardware */
	init_catgenie();
	PORTB_old = PORTB;

	/* initialize timer 1 */
	timer_init();

	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}

void main (void)
{
	unsigned long	delay = SECOND/2;
	struct timer	test_timer ;

	/* Initialize the hardware */
	init();

	settimeout(&test_timer, delay);

	/* Execute the run loop */
	for(;;){
		do_catgenie();
		if (timeoutexpired(&test_timer))
		{
			settimeout(&test_timer, delay);
			test++;
			set_LED_Error(test & 1);
		}
	}
}

static void interrupt isr (void)
{
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
		timer_isr();
	}
	if (INTF) {
		/* Reset interrupt */
		INTF = 0;
		/* Handle interrupt */
		startbutton_isr();
	}
	if (RBIF) {
		unsigned char	PORTB_diff = PORTB ^ PORTB_old;
		/* Reset interrupt */
		RBIF = 0;
		/* Handle interrupt */
		if (PORTB_diff & 0x10)
			catsensor_isr();
		if (PORTB_diff & 0x20)
			setupbutton_isr();
		PORTB_old = PORTB ;
	}
}
