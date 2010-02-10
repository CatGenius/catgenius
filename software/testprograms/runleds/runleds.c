#include <htc.h>

#include "timer.h"

__CONFIG(XT & WDTDIS & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);
//__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);

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
	/* Initialize the hardware */
	init();

	/* Execute the run loop */
	for(;;){
	}
}