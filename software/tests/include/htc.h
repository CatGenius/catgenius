/* Host-only registers and declarations, not a PIC ABI or electrical model. */
#ifndef CATGENIUS_TEST_HTC_H
#define CATGENIUS_TEST_HTC_H

typedef _Bool bit;

extern volatile unsigned char PORTA, PORTB, PORTC, PORTD, PORTE;
extern volatile unsigned char LATA, LATB, LATC, LATD, LATE;
extern volatile unsigned char TRISA, TRISB, TRISC, TRISD, TRISE;
extern volatile unsigned char ANSELA, ANSELB, ANSELD, ANSELE;
extern volatile unsigned char WPUB, WPUE, nWPUEN, nRBPU;
extern volatile unsigned char RBIF, RBIE, IOCBP, IOCBN, IOCBF, IOCIF, IOCIE;
extern volatile unsigned char ADCON1;
extern volatile unsigned int ADRES;
extern volatile unsigned char TMR4, PR4, T4CON, TMR4IE, TMR4IF, GIE;

struct host_adcon0 {
	unsigned char ADON;
	unsigned char CHS;
	unsigned char GO;
};
struct host_adcon1 {
	unsigned char ADFM;
	unsigned char ADCS;
	unsigned char ADNREF;
	unsigned char ADPREF;
};
extern volatile struct host_adcon0 ADCON0bits;
extern volatile struct host_adcon1 ADCON1bits;
/* GO and nDONE name the same hardware bit. */
#define nDONE GO

unsigned char eeprom_read(unsigned char address);
void eeprom_write(unsigned char address, unsigned char value);
void host_delay_ms(unsigned int duration);
#define __delay_ms(duration) host_delay_ms(duration)

#endif
