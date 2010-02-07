#include <htc.h>

__CONFIG(XT & WDTEN & PWRTEN & BOREN & LVPDIS & DPROT & WRTEN & PROTECT);

void init (void)
{
}

void main (void)
{
	/* Initialize the hardware */
	init();

	/* Execute the run loop */
	for(;;){
	}
}