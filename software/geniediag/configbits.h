/******************************************************************************/
/* File    :	configbits.h						      */
/* Function:	Sets the PIC MCU's configuration bits.			      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2018, Clockwork Engineering		      */
/******************************************************************************/

#ifndef CONFIGBITS_H				/* Include file already compiled? */
#  define CONFIGBITS_H

#if (defined __PICC__)
#  if (defined _16F877A)
#    ifdef __DEBUG
	__CONFIG(FOSC_XT & WDTE_OFF & PWRTE_ON & BOREN_OFF & LVP_OFF & CPD_OFF & WRT_OFF & DEBUG_ON  & CP_OFF);
#    else
	__CONFIG(FOSC_XT & WDTE_ON  & PWRTE_ON & BOREN_ON  & LVP_OFF & CPD_ON  & WRT_OFF & DEBUG_OFF & CP_ON);
#    endif
#  elif (defined _16F1939)
#    ifdef __DEBUG
	__CONFIG(FOSC_XT & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & CPD_OFF & BOREN_OFF & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
	__CONFIG(WRT_OFF & VCAPEN_OFF & PLLEN_OFF & STVREN_ON & BORV_HI & LVP_OFF);
#    else
	__CONFIG(FOSC_XT & WDTE_ON  & PWRTE_ON  & MCLRE_ON & CP_ON  & CPD_ON  & BOREN_ON  & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
	__CONFIG(WRT_OFF & VCAPEN_OFF & PLLEN_OFF & STVREN_ON & BORV_HI & LVP_OFF);
#    endif
#  endif
#elif (defined __XC8)
#  pragma config FOSC = XT		/* Oscillator Selection (XT Oscillator, Crystal/resonator connected between OSC1 and OSC2 pins) */
#  pragma config LVP = OFF		/* Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming) */
#  pragma config WRT = OFF		/* Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control) */
#  if (defined _16F877A)
#    pragma config PWRTE = ON		/* Power-up Timer Enable bit (PWRT enabled) */
#    ifdef __DEBUG
#      pragma config WDTE = OFF		/* Watchdog Timer Enable bit (WDT disabled) */
#      pragma config BOREN = OFF	/* Brown-out Reset Enable bit (BOR disabled) */
#      pragma config CPD = OFF		/* Data EEPROM Memory Code Protection bit (Data EEPROM code protection off) */
#      pragma config CP = OFF		/* Flash Program Memory Code Protection bit (Code protection off) */
#    else
#      pragma config WDTE = ON		/* Watchdog Timer Enable bit (WDT enabled) */
#      pragma config BOREN = ON		/* Brown-out Reset Enable bit (BOR enabled) */
#      pragma config CPD = ON		/* Data EEPROM Memory Code Protection bit (Data EEPROM code-protected) */
#      pragma config CP = ON		/* Flash Program Memory Code Protection bit (All program memory code-protected) */
#    endif
#  elif (defined _16F1939)
#    pragma config MCLRE = ON		/* MCLR Pin Function Select (MCLR/VPP pin function is MCLR) */
#    pragma config CLKOUTEN = OFF	/* Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin) */
#    pragma config IESO = OFF		/* Internal/External Switchover (Internal/External Switchover mode is disabled) */
#    pragma config FCMEN = OFF		/* Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled) */
#    ifdef __DEBUG
#      pragma config WDTE = OFF		/* Watchdog Timer Enable (WDT disabled) */
#      pragma config PWRTE = OFF	/* Power-up Timer Enable (PWRT disabled) */
#      pragma config CP = OFF		/* Flash Program Memory Code Protection (Program memory code protection is disabled) */
#      pragma config CPD = OFF		/* Data Memory Code Protection (Data memory code protection is disabled) */
#      pragma config BOREN = OFF	/* Brown-out Reset Enable (Brown-out Reset disabled) */
#    else
#      pragma config WDTE = ON		/* Watchdog Timer Enable (WDT enabled) */
#      pragma config PWRTE = ON		/* Power-up Timer Enable (PWRT enabled) */
#      pragma config CP = ON		/* Flash Program Memory Code Protection (Program memory code protection is enabled) */
#      pragma config CPD = ON		/* Data Memory Code Protection (Data memory code protection is enabled) */
#      pragma config BOREN = ON		/* Brown-out Reset Enable (Brown-out Reset enabled) */
#    endif
#    pragma config VCAPEN = OFF		/* Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled) */
#    pragma config PLLEN = OFF		/* PLL Enable (4x PLL disabled) */
#    pragma config STVREN = ON		/* Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset) */
#    pragma config BORV = HI		/* Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), high trip point selected.) */
#  endif
#else
#  error Unsupported toolchain
#endif


#else
#  error This file should be included only once, as the very first include in the main application
#endif /* CONFIGBITS_H */
