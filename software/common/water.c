/******************************************************************************/
/* File    :	water.c							      */
/* Function:	Water valve and sensor functional implementation	      */
/* Author  :	Robert Delien						      */
/*		Copyright (C) 2010, Clockwork Engineering		      */
/* History :	30 Dec 2012 by R. Delien:				      */
/*		- Renamed from watersensor.c.				      */
/******************************************************************************/
#include <xc.h>

#include "hardware.h"			/* Flexible hardware configuration */

#include "water.h"
#include "timer.h"
#ifdef WATERSENSOR_ANALOG
#include "waterquality.h"
#endif


extern void waterdetection_event	(unsigned char	detected);
extern void watersensor_event		(unsigned int	reflectionquality);


/******************************************************************************/
/* Macros								      */
/******************************************************************************/

#define DETECTTIME		(SECOND/1000)	/*   1ms*/
#define CONVERSION_TIMEOUT	(SECOND/100)	/*  10ms watchdog, not an acquisition delay */
#ifdef WATERSENSOR_ANALOG
#define WATERSENSORPOLLING	(SECOND/10)	/* 100ms between normal cycle starts */
#define HYSTERESIS_MAX		6		/* Consecutive comparator samples */
#define QUALITY_SETTLE		(SECOND/40)	/*  25ms with the IR LED on */
#define ADC_ACQUISITION		((SECOND + 39999)/40000) /* At least 25us */
#define PROBE_COUNTS		(_XTAL_FREQ/4/4/2000) /* Timer4: Fosc/4, prescale 4, 0.5ms */
#if (PROBE_COUNTS < 1) || (PROBE_COUNTS > 256)
#error Timer4 probe period is out of range
#endif
#else
#define WATERSENSORPOLLING	(SECOND/4)	/* 250ms*/
#define HYSTERESIS_MAX		8		/* Number of pollings to debounce the sensor output */
#endif

/*
 * The LM393 inverting schmitt-trigger circuit shuts the water valve autonomously
 * when the light guide is submerged. This circuit is a safe guard against
 * overflows that will work regardless of the state of software.
 * Using a multi-turn poteniometer, we have determined that an open water valve
 * will be closed by the LM393 at a value of 519-520. Due to a little hysteresis,
 * it will open again when the value lowers down to 504-503. The average of these
 * two switch points is therefore: (519+504)/2=1023/2=511,5. With a 10-bits A/D-
 * converter yielding a span of 0..1023, this value is spot-on in the middle.
 */
#define DETECTION_THRESHOLD	520		/* At an ADC value of 520 or above, the LM393 closes the water valve */
#define UNDETECTION_THRESHOLD	503		/* At an ADC value of 503 or below, the LM393 opens the water valve */

/*
 * After the LM393 has closed the water valve, naturally the water level
 * will no longer rise, hence the analog reflection quality value will no
 * longer rise. In theory, a wave of water could briefly trigger the LM393
 * to close the valve, without notifying software with a washing program
 * waiting for high water level. To avoid this race condition, software
 * should be notified at a the highest value at which the water valve is
 * still open (UNDETECTION_THRESHOLD).  Just to be sure, an extra safety
 * margin is subtracted. The hysteresis span seems appropriate.
 */
#define DETECTION_MARGIN	((DETECTION_THRESHOLD)-(UNDETECTION_THRESHOLD))
 
 
 
#define LED_ON			0
#define START_CONVERSION	1
#define PROCESS_RESULT		2
#define WAIT_PROBE		3


/******************************************************************************/
/* Global Data								      */
/******************************************************************************/

static struct timer	sensortimer       = EXPIRED;
static unsigned char	state             = 0;
#ifndef WATERSENSOR_ANALOG
static unsigned char	hysteresis        = 0;
#endif
static unsigned char	samples           = 0;
static unsigned int	reflectionquality = 0;
static __bit		valid             = 0;
static __bit		failed            = 0;
static volatile __bit	filling           = 0;
static __bit		detected          = 0;
static volatile __bit	ledalwayson       = 0;

#ifdef WATERSENSOR_ANALOG
static struct timer	polltimer         = EXPIRED;
static struct waterquality quality;
static unsigned int	adc_sum           = 0;
static unsigned char	adc_samples       = 0;
static unsigned char	acquisition_fault = WATER_ACQUISITION_OK;
static __bit		level_candidate   = 0;
static __bit		sample_filling    = 0;
static __bit		reflection_filling= 0;
static __bit		reflection_valid  = 0;
static __bit		comparator        = 0;
static __bit		comparator_valid  = 0;
static volatile __bit	probe_done        = 0;
static volatile __bit	probe_high        = 0;
static volatile __bit	probe_keep_led    = 0;
#endif


/******************************************************************************/
/* Local Prototypes							      */
/******************************************************************************/

#ifdef WATERSENSOR_ANALOG
static void	water_work_analog	(void);
static void	cancel_acquisition	(void);
static void	fail_acquisition	(unsigned char fault);
static void	finish_batch		(unsigned char have_probe);
#endif

/******************************************************************************/
/* Global Implementations						      */
/******************************************************************************/

void water_init (void)
/******************************************************************************/
/* Function:	Module initialisation routine				      */
/*		- Initializes the module				      */
/* History :	16 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
#ifdef WATERSENSOR_ANALOG
	unsigned char	mask    = WATERSENSORANALOG_MASK;
	unsigned char	channel = 0;

	/* Timer4 belongs to the water comparator probe, not the cat-sensor PWM. */
	TMR4IE = 0;
	T4CON = 0;
	TMR4IF = 0;
	PR4 = PROBE_COUNTS - 1;
	state = LED_ON;
	samples = adc_samples = 0;
	adc_sum = reflectionquality = 0;
	valid = failed = filling = detected = ledalwayson = 0;
	level_candidate = sample_filling = reflection_filling = 0;
	reflection_valid = 0;
	comparator = comparator_valid = probe_done = probe_high = probe_keep_led = 0;
	acquisition_fault = WATER_ACQUISITION_OK;
	waterquality_init(&quality, WATER_QUALITY_THRESHOLD);
	WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
	WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
	timeoutnow(&polltimer);
	ADCON0bits.GO = 0;

	/* Dynamically determine channel# from mask */
	while (!(mask & 0x01)) {
		mask >>= 0x01;
		channel ++;
	}

	/* Power-up AD circuitry */
	ADCON0bits.ADON = 1;
	/* Select input channel */
	ADCON0bits.CHS = channel;

	/* Set output format to right-justified data */
	ADCON1bits.ADFM = 1;
	/* Fosc/32 gives 8us per ADC clock at 4MHz. */
	ADCON1bits.ADCS = 2;

	/* Set negative reference to Vss, positive reference to Vdd */
	ADCON1bits.ADNREF = 0;
	ADCON1bits.ADPREF = 0;
#endif /* WATERSENSOR_ANALOG */
}
/* End: water_init */


void water_work (void)
/******************************************************************************/
/* Function:	Module worker routine					      */
/*		- Worker function for the CatGenie 120 water sensor and valve */
/* History :	12 Feb 2010 by R. Delien:				      */
/*		- Initial revision.					      */
/******************************************************************************/
{
#ifdef WATERSENSOR_ANALOG
	water_work_analog();
#else
	unsigned int	cur_reflectionquality;

	switch (state) {
	default:
		state = LED_ON;
	case LED_ON:
		if (!timeoutexpired(&sensortimer))
			break;
		/* Switch on the IR LED */
		WATERSENSOR_LED(LAT) |= WATERSENSOR_LED_MASK;
		/* Wait for DETECTTIME to give the IR sensor some time */
		settimeout(&sensortimer, DETECTTIME);
		state = PROCESS_RESULT;
		break;
	case PROCESS_RESULT:
		if (!timeoutexpired(&sensortimer))
			break;
		/* Read out the IR sensor digitally (lower value == more light reflected == no water detected) */
		cur_reflectionquality = (WATERSENSORANALOG(PORT) & WATERSENSORANALOG_MASK)?DETECTION_THRESHOLD:0;
		/* Switch off the IR LED if we're not filling */
		if (!filling && !ledalwayson)
			WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
		/* Evaluate the result, considering a hysteresis */
		if (cur_reflectionquality <= (UNDETECTION_THRESHOLD - DETECTION_MARGIN)) {
			if ((hysteresis > 0) &&
			    (!--hysteresis && detected)) {
				detected = 0;
				waterdetection_event(detected);
			}
		} else {
			if ((hysteresis < HYSTERESIS_MAX) &&
			    (++hysteresis >= HYSTERESIS_MAX) && !detected) {
				detected = 1;
				waterdetection_event(detected);
			}
		}
		/* Qualify startup and recovery before trusting the debounced level. */
		if (samples < HYSTERESIS_MAX)
			samples++;
		if ((samples == HYSTERESIS_MAX) &&
		    ((hysteresis == 0) || (hysteresis == HYSTERESIS_MAX))) {
			valid = 1;
			failed = 0;
		}
		/* Check water sensor reflection quality */
		if (cur_reflectionquality != reflectionquality) {
			reflectionquality = cur_reflectionquality;
			watersensor_event(cur_reflectionquality);
		}

		settimeout(&sensortimer, WATERSENSORPOLLING);
		state = LED_ON;
		break;
	}
#endif /* WATERSENSOR_ANALOG */
}
/* End: water_work */


unsigned char water_detected (void)
{
	return (detected);
}
/* End: water_detected */


unsigned char water_valid (void)
{
	return (valid);
}
/* End: water_valid */


unsigned char water_failed (void)
{
	return (failed);
}
/* End: water_failed */


unsigned int water_reflectionquality (void)
{
	return (reflectionquality);
}
/* End: water_reflectionquality */


void water_ledalwayson (unsigned char on)
{
	ledalwayson = on ? 1 : 0;
#ifdef WATERSENSOR_ANALOG
	if (ledalwayson)
		WATERSENSOR_LED(LAT) |= WATERSENSOR_LED_MASK;
	else if (!filling && (state == LED_ON))
		WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
#endif
}
/* End: water_ledalwayson */


unsigned char water_filling (void)
{
	return (filling);
}
/* End: water_filling */


void water_fill (unsigned char fill)
{
#ifdef WATERSENSOR_ANALOG
	/* A change of fill state must not mix acquisition contexts or leave a probe on. */
	cancel_acquisition();
	filling = (fill && !failed &&
		   (quality.status != WATER_QUALITY_CHECKING) &&
		   (quality.status != WATER_QUALITY_OPTICAL) &&
		   (quality.status != WATER_QUALITY_LEVEL)) ? 1 : 0;
	if (filling) {
		WATERSENSOR_LED(LAT) |= WATERSENSOR_LED_MASK;
		WATERVALVEPULLUP(LAT) |= WATERVALVEPULLUP_MASK;
	} else {
		WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
		if (!ledalwayson)
			WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
	}
#else
	/* Recovery requires new qualified samples and an explicit fill request. */
	filling = (fill && !failed) ? 1 : 0;

	if (filling) {
		/* Pull-up WATERVALVE */
		WATERVALVEPULLUP(LAT) |= WATERVALVEPULLUP_MASK;
	} else {
		/* Pull-down WATERVALVE */
		WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
	}
#endif /* WATERSENSOR_ANALOG */
}
/* End: water_fill */


#ifdef WATERSENSOR_ANALOG
void water_isr (void)
{
	/* No callbacks, shared helper calls or serial output in this ISR. */
	T4CON = 0;
	TMR4IE = 0;
	probe_high = (WATERVALVE(PORT) & WATERVALVE_MASK) ? 1 : 0;
	if (!filling)
		WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
	if (!filling && !ledalwayson && !probe_keep_led)
		WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
	TMR4IF = 0;
	/* Publish only after the valve-enable pulse has ended. */
	probe_done = 1;
}

unsigned char water_check (unsigned char probe)
{
	if (filling || get_Pump() || get_Dosage() || get_Dryer() ||
	    (get_Bowl() != BOWL_STOP) || (get_Arm() != ARM_STOP))
		return 0;
	if (!waterquality_begin(&quality, probe))
		return 0;
	cancel_acquisition();
	WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
	valid = comparator_valid = 0;
	samples = 0;
	timeoutnow(&polltimer);
	return 1;
}

void water_check_cancel (void)
{
	if (quality.status == WATER_QUALITY_CHECKING)
		quality.status = WATER_QUALITY_UNCHECKED;
	water_fill(0);
}

unsigned char water_quality (void)
{
	return (quality.status);
}
/* End: water_quality */


unsigned char water_acquisition_fault (void)
{
	return (acquisition_fault);
}
/* End: water_acquisition_fault */


unsigned char water_comparator (void)
{
	return (comparator);
}
/* End: water_comparator */


unsigned char water_comparator_valid (void)
{
	return (comparator_valid);
}
/* End: water_comparator_valid */


unsigned char water_reflection_filling (void)
{
	return (reflection_filling);
}
/* End: water_reflection_filling */

unsigned char water_reflection_valid (void)
{
	return (reflection_valid);
}
/* End: water_reflection_valid */


#endif /* WATERSENSOR_ANALOG */


/******************************************************************************/
/* Local Implementations						      */
/******************************************************************************/

#ifdef WATERSENSOR_ANALOG
static void cancel_acquisition (void)
{
	/* Disabling the source first also excludes a pending probe ISR. */
	TMR4IE = 0;
	T4CON = 0;
	TMR4IF = 0;
	ADCON0bits.GO = 0;
	probe_done = probe_keep_led = 0;
	adc_sum = 0;
	adc_samples = 0;
	state = LED_ON;
	settimeout(&polltimer, WATERSENSORPOLLING);
}

static void fail_acquisition (unsigned char fault)
{
	failed = 1;
	acquisition_fault = fault;
	valid = comparator_valid = 0;
	samples = 0;
	/* A fault breaks consecutive recovery and confirmation evidence. */
	quality.good = 0;
	quality.repeats = quality.analog_bad = quality.comparator_bad = 0;
	/* Inhibit the valve before extinguishing illumination. */
	water_fill(0);
}

static void finish_batch (unsigned char have_probe)
{
	unsigned int	old_reflectionquality = reflectionquality;
	unsigned char	old_detected = detected;
	unsigned char	old_quality = quality.status;
	unsigned char	level;

	reflectionquality = adc_sum >> 2;
	reflection_filling = sample_filling;
	reflection_valid = 1;
	if (have_probe) {
		comparator = probe_high;
		comparator_valid = 1;
	}
	probe_done = 0;
	if ((old_quality == WATER_QUALITY_CHECKING) ||
	    (old_quality == WATER_QUALITY_OPTICAL)) {
		waterquality_batch(&quality, reflectionquality, comparator);
		/* Quality batches are not normal, regularly spaced level samples. */
		valid = 0;
		samples = 0;
	} else if (have_probe) {
		level = comparator ? 0 : 1;
		if (!samples || (level_candidate != level)) {
			level_candidate = level;
			samples = 1;
		} else if (samples < HYSTERESIS_MAX)
			samples++;
		if (samples == HYSTERESIS_MAX) {
			detected = level;
			valid = 1;
			failed = 0;
			acquisition_fault = WATER_ACQUISITION_OK;
		}
	}
	state = LED_ON;
	/* Keep illumination through confirmation, but never keep its probe enabled. */
	if (quality.status == WATER_QUALITY_CHECKING)
		timeoutnow(&polltimer);
	else if (!filling && !ledalwayson)
		WATERSENSOR_LED(LAT) &= ~WATERSENSOR_LED_MASK;
	if (old_detected != detected)
		waterdetection_event(detected);
	if (old_reflectionquality != reflectionquality)
		watersensor_event(reflectionquality);
}

static void water_work_analog (void)
{
	unsigned char	interrupts_enabled;

	switch (state) {
	case LED_ON:
		if (!timeoutexpired(&polltimer))
			break;
		settimeout(&polltimer, WATERSENSORPOLLING);
		/* A quality check/recovery always measures with filling inhibited. */
		if (!filling)
			WATERVALVEPULLUP(LAT) &= ~WATERVALVEPULLUP_MASK;
		WATERSENSOR_LED(LAT) |= WATERSENSOR_LED_MASK;
		sample_filling = filling;
		adc_sum = 0;
		adc_samples = 0;
		settimeout(&sensortimer, QUALITY_SETTLE);
		state = START_CONVERSION;
		break;
	case START_CONVERSION:
		if (!timeoutexpired(&sensortimer))
			break;
		ADCON0bits.GO = 1;
		settimeout(&sensortimer, CONVERSION_TIMEOUT);
		state = PROCESS_RESULT;
		break;
	case PROCESS_RESULT:
		if (ADCON0bits.nDONE) {
			if (timeoutexpired(&sensortimer))
				fail_acquisition(WATER_ACQUISITION_ADC);
			break;
		}
		adc_sum += ADRES;
		if (++adc_samples < 4) {
			/* Give the hold capacitor a fresh acquisition interval each time. */
			settimeout(&sensortimer, ADC_ACQUISITION);
			state = START_CONVERSION;
			break;
		}
		if (((quality.status == WATER_QUALITY_CHECKING) && !quality.probe) ||
		    (quality.status == WATER_QUALITY_OPTICAL) ||
		    (quality.status == WATER_QUALITY_LEVEL)) {
			/* Recovery and analogue-only checking must not pulse the valve. */
			finish_batch(0);
			break;
		}
		probe_done = 0;
		probe_keep_led = (quality.status == WATER_QUALITY_CHECKING) ? 1 : 0;
		settimeout(&sensortimer, CONVERSION_TIMEOUT);
		state = WAIT_PROBE;
		TMR4IE = 0;
		T4CON = 0;
		TMR4 = 0;
		TMR4IF = 0;
		/* Do not allow an intervening ISR before the pulse timer is started. */
		interrupts_enabled = GIE;
		GIE = 0;
		WATERVALVEPULLUP(LAT) |= WATERVALVEPULLUP_MASK;
		T4CON = 0x05;	/* Prescale 4, postscale 1, timer on */
		TMR4IE = 1;
		GIE = interrupts_enabled;
		break;
	case WAIT_PROBE:
		if (probe_done)
			finish_batch(1);
		else if (timeoutexpired(&sensortimer))
			fail_acquisition(WATER_ACQUISITION_PROBE);
		break;
	default:
		fail_acquisition(WATER_ACQUISITION_PROBE);
		break;
	}
}
#endif /* WATERSENSOR_ANALOG */
