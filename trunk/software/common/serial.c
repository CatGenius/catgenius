#include <htc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#if ((_XTAL_FREQ/(16UL * BITRATE))-1UL) < 256UL
#  define SPBRG_SET (_XTAL_FREQ/(16UL * BITRATE))-1
#  define BRGH_SET 1
#else
#  define SPBRG_SET (_XTAL_FREQ/(64UL * BITRATE))-1
#  define BRGH_SET 0
#endif

#define BITRATE_REAL	(_XTAL_FREQ/((64UL-(BRGH_SET*48UL))*(SPBRG_SET+1)))
#if BITRATE_REAL > BITRATE
#  if (((BITRATE_REAL - BITRATE)*100UL)/BITRATE) > BITRATE_ERROR
#    error Bit rate error is too high
#  endif
#else
#  if (((BITRATE - BITRATE_REAL)*100UL)/BITRATE) > BITRATE_ERROR
#    error Bit rate error is too high
#  endif
#endif




void serial_init(void)
{
	/*
	 * Comms setup:
	 */
#if _XTAL_FREQ==3686400 && BITRATE==19200
#  assert SPBRG_SET==11
#elif _XTAL_FREQ==4000000 && BITRATE==19200
#  assert SPBRG_SET==12
#elif _XTAL_FREQ==10000000 && BITRATE==19200
#  if BRGH_SET==0
#    assert SPBRG_SET==7
#  elif BRGH_SET==1
#    assert SPBRG_SET==31
#  else
#    error
#  endif
#elif _XTAL_FREQ==10000000 && BITRATE==33600
#  if BRGH_SET==0
#    assert SPBRG_SET==4
#  elif BRGH_SET==1
#    assert SPBRG_SET==18
#  else
#    error
#  endif
#elif _XTAL_FREQ==16000000 && BITRATE==19200
#  assert SPBRG_SET==51
#elif _XTAL_FREQ==20000000 && BITRATE==19200
#  assert SPBRG_SET==64
#elif _XTAL_FREQ==20000000 && BITRATE==57600
#  if BRGH_SET==0
#    assert SPBRG_SET==4
#  elif BRGH_SET==1
#    assert SPBRG_SET==20
#  else
#    error
#  endif
#else
#  error could not #assert SPRBG and BRGH for specified crystal freq. and baud rate
#endif
	SPBRG=SPBRG_SET;
	BRGH=BRGH_SET;	//data rate for sending/receiving
	SYNC=0;		//asynchronous
	SPEN=1;		//enable serial port pins
	CREN=1;		//enable reception
	SREN=0;		//no effect
	TXIE=0;		//disable tx interrupts
	RCIE=0;		//disable rx interrupts
	TX9=0;		//8-bit transmission
	RX9=0;		//8-bit reception
	TXEN=0;		//reset transmitter
	TXEN=1;		//enable the transmitter
}

unsigned char dummy;

#define clear_usart_errors_inline	\
if (OERR)				\
{					\
	TXEN=0;				\
	TXEN=1;				\
	CREN=0;				\
	CREN=1;				\
}					\
if (FERR)				\
{					\
	dummy=RCREG;			\
	TXEN=0;				\
	TXEN=1;				\
}

//writes a character to the serial port
void putch(unsigned char c)
{
	while(!TXIF)			//set when register is empty
	{
		clear_usart_errors_inline;
		CLRWDT();
	}
	TXREG=c;
	__delay_us(600);
}

unsigned char readch(char *ch)
{
	if (!RCIF)
		return (0);

	if (OERR) {
		TXEN = 0;
		TXEN = 1;
		CREN = 0;
		CREN = 1;
		return (0);
	}
	if (FERR) {
		*ch  = RCREG;
		TXEN = 0;
		TXEN = 1;
		return (0);
	}

	*ch  = RCREG;
	return (1);
}

unsigned char getch_timeout(void)
{
#if 0
	unsigned char i;
	unsigned int timeout_int;

	// retrieve one byte with a timeout
	for (i=2;i!=0;i--)
	{
		timeout_int=timeout_int_us(240000);

		while (hibyte(timeout_int)!=0) //only check the msb of the int for being 0, it saves space, see always.h for macro
		{
			CLRWDT();
			timeout_int--;
			if (RCIF)
			{
				return RCREG;
			}
		}
	}
#endif
	return 0;
}


void clear_usart_errors(void)
{
	clear_usart_errors_inline;
}



/*
writes a character to the serial port in hex
if serial lines are disconnected, there are no errors
*/

void putchhex(unsigned char c)
{
	unsigned char temp;

	temp = c >> 4;

	if (temp < 10)
		temp += '0';
	else
		temp += ('A' - 10);
	putch(temp);

	temp = c & 0x0F;
	if (temp < 10)
		temp += '0';
	else
		temp += ('A' - 10);
	putch(temp);
}

void putinthex(unsigned int c)
{
	#define ramuint(x)						(*((unsigned int *) (x)))
	#define ramuint_hibyte(x)			(*(((unsigned char *)&x)+1))
	#define ramuint_lobyte(x)			(*(((unsigned char *)&x)+0))
	#define ramuchar(x)						(*((unsigned char *) (x)))

	putchhex(ramuint_hibyte(c));
	putchhex(ramuint_lobyte(c));

	#undef ramuint(x)
	#undef ramuint_hibyte(x)
	#undef ramuint_lobyte(x)
	#undef ramuchar(x)
}

//if there has been a previous timeout error from getch_timeout, this returns TRUE
unsigned char usart_timeout(void)
{
//	return usart_timeout_error;
	return 0;
}

/*
writes a character to the serial port in decimal
if serial lines are disconnected, there are no errors
*/
void putchdec(unsigned char c)
{
	unsigned char temp;

	temp=c;
	//hundreds
	if ((c/100)>0) putch((c/100)+'0');
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) putch((c/10)+'0');
	c-=(c/10)*10;

	//ones
	putch((c/1)+'0');
}

void putst(register const char *str)
{
	while((*str)!=0)
	{
		putch(*str);
		if (*str==13) putch(10);
		if (*str==10) putch(13);
		str++;
	}
}

