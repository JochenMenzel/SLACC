#include "pwm.h"
//#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

uint8_t pwm;

uint8_t pwm_min;
uint8_t pwm_max;

void pwm_init(void)
{
    // configure shutdown of 0� buck stage as output
    PWM_SHTDN_0_DDR |= 1 << PWM_SHTDN_0_BIT;
    //configure shutdown of 180� buck stage as output
    PWM_SHTDN_180_DDR |= 1 << PWM_SHTDN_180_BIT;

    // configure pwm pins as output
    DDRD |= (1 << DDD5) | (1 << DDD3); // OC0B, OC2B

    // Timer/Counter2 Interrupt Mask Register
    TIMSK0 = timer0_Interrupt_none;    
    TIMSK2 = timer2_Interrupt_none;

    pwm = PWM_BOTTOM;
    // Output Compare Register B
    OCR0B = OCR2B = pwm;
    // Output Compare Register A
//    OCR0A = OCR2A = PWM_TOP;

    //set temporary values for the global limits pwm_min and pwm_max
    pwm_max = PWM_HIGH_F_MAX;
    pwm_min = PWM_HIGH_F_MIN;
    
    //stop and clear pwm setup in timers and hardware
    pwm_0deg_disable();
    pwm_180deg_disable();
}


// Insecure! Do not use directly (does not ensure bootstrapping)
void pwm_set(uint8_t value)
{
    if (value > PWM_TOP)
        value = PWM_TOP;
    pwm = value;
    OCR0B = OCR2B = value;
}


// returns 1 if pwm could not be increased
uint8_t pwm_stepUp(void)
{
    uint8_t ret = 0;
    if (pwm >= pwm_max)
    {
        pwm = pwm_max;
        ret = 1;
    }
    else if (pwm_max - pwm >= PWM_STEP)
        pwm += PWM_STEP;
    else
        pwm = pwm_max;
        
    OCR0B = OCR2B = pwm;
    
    return ret;
}


// returns 1 if pwm could not be decreased
uint8_t pwm_stepDown(void)
{
    uint8_t ret = 0;
    if (pwm <= pwm_min)
    {
        pwm = pwm_min;
        ret = 1;
    }
    else if (pwm - pwm_min >= PWM_STEP)
        pwm -= PWM_STEP;
    else
        pwm = pwm_min;
        
    OCR0B = OCR2B = pwm;
    
    return ret;
}

uint8_t pwm_get(void)
{
    return pwm;
}

/*
 * this function stops the 0�-phase buck stage attached to timer 0�s OC0B pwm output on pin PD5.
 */
void pwm_0deg_disable(void)
{
    // _shutdown to low
    PWM_SHTDN_0_PORT &= ~(1 << PWM_SHTDN_0_BIT);
    // pwm pins to low
    PORTD &= ~(1 << DDD5); // OC0B

    // set timer to default and stop clock
    TCCR0A =  0;
    TCCR0B =  0;
}

/*
 * this function stops the 180�-phase buck stage attached to timer 2�s OC2B pwm output on pin PD3.
 */
void pwm_180deg_disable(void)
{
    // _shutdown to low
    PWM_SHTDN_180_PORT &= ~(1 << PWM_SHTDN_180_BIT);
    // pwm pins to low
    PORTD &= ~(1 << DDD3); // OC0B, OC2B

    // set timers to defaults and stop clock
    TCCR2A = 0;
    TCCR2B = 0;
}

/*
 * this function enables the 0� PWM signal on OC0B (pin PD5) and enables the driver for the 0� buck power stage.
 * param pwm_frequency_t pwm_frequency selects 976Hz, 62.5 kHz or 125 kHz.
 * Attention: NEVER EVER start 0� PWM with a different frequency setting than that of the 180� PWM!
 */
void pwm_0deg_enable(pwm_frequency_t pwm_frequency)
{
	//do not interrupt this! -> disable interrupts
    cli();

    // TCCR0A – Timer/Counter Control Register A
    TCCR0A = timer0_CompareMatchOutputAMode_normal
           | timer0_CompareMatchOutputBMode_clear // use OC0B for unshifted shifted pwm
           | timer0_WaveformGenerationModeA_pwmFastOcr;

    switch (pwm_frequency) {
    	case pwm_frequencyMinimal:
    	case pwm_frequencyMedium:
    	    // Set top
    	    OCR0A = PWM_MEDIUM_F_TOP;
    	    //set pwm value limits
    	    pwm_min = PWM_MEDIUM_F_MIN;
    	    pwm_max = PWM_MEDIUM_F_MAX;
    	break;

    	case pwm_frequencyHigh:
    	default:
    	    // Set top
    	    OCR0A = PWM_HIGH_F_TOP;
    	    // set pwm value limits
    	    pwm_min = PWM_HIGH_F_MIN;
    	    pwm_max = PWM_HIGH_F_MAX;
    	break;
    }

    // Timer/Counter Register
    TCNT0 = 0; // Timer0 - no phase shift

    switch (pwm_frequency) {
    	case pwm_frequencyMinimal:
			// TCCR0B – Timer/Counter Control Register B
			TCCR0B = timer0_ForceOutputCompare_none
				   | timer0_WaveformGenerationModeB_pwmFastOcr
				   | timer0_ClockSelect_div64;
		break;

    	case pwm_frequencyMedium:
    	case pwm_frequencyHigh:
    	default:
    		// TCCR0B – Timer/Counter Control Register B
			TCCR0B = timer0_ForceOutputCompare_none
				   | timer0_WaveformGenerationModeB_pwmFastOcr
				   | timer0_ClockSelect_1;
		break;
    };

    // _shutdown high enables MOSFET-driver
    PWM_SHTDN_0_PORT |= 1 << PWM_SHTDN_0_BIT;

    //enable interrupts
    sei();
}

/*
 * this function enables the 180� PWM signal on OC2B (pin PD3) and enables the driver for the 180� buck power stage.
 * param pwm_frequency_t pwm_frequency selects 976Hz, 62.5 kHz or 125 kHz.
 * Attention: NEVER EVER start 180� PWM with a different frequency setting than that of the 0� PWM!
 * Attention: NEVER EVER start 180� PWM when 0� PWM is stopped!
 */
void pwm_180deg_enable(pwm_frequency_t pwm_frequency)
{
	// we want to shortly stop timer 0 and restart it in sync with timer 2. Therefore, we
	// use a temporary storage for timer0's control register B.
	uint16_t timer0_controlregisterB;

	//do not interrupt this! -> disable interrupts
    cli();

    // store setup of timer 0
    timer0_controlregisterB = TCCR0B;
    // stop timer 0 temporarily
    TCCR0B = 0;

    //reset the prescalers and keep both prescalers (and thus all timers) stopped
    GTCCR = (1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);

    // TCCR2A – Timer/Counter Control Register A
    TCCR2A = timer2_CompareMatchOutputAMode_normal
           | timer2_CompareMatchOutputBMode_clear // use OC2B for 180 deg shifted pwm
           | timer2_WaveformGenerationModeA_pwmFastOcr;
    
    switch (pwm_frequency) {
    	case pwm_frequencyMinimal:
    	case pwm_frequencyMedium:
    	    // Set top to configuration for medium and minimal frequency
    	    OCR2A = PWM_MEDIUM_F_TOP;
    	    //set pwm value limits
    	    pwm_min = PWM_MEDIUM_F_MIN;
    	    pwm_max = PWM_MEDIUM_F_MAX;
    	    // Timer2 shall run 180 deg phase shifted in relation to timer0. Since timer 0
    	    // may run with 7 bit or 8 bit resolution depending on timer clock frequency,
    	    // we use half of the timer 2 TOP value from OCR2A to initialize timer 2.
    	    TCNT2 = (PWM_MEDIUM_F_TOP >> 1);
    	    break;

    	case pwm_frequencyHigh:
    	default:
    	    // Set top to configuration for high frequency
    	    OCR2A = PWM_HIGH_F_TOP;
    	    // set pwm value limits
    	    pwm_min = PWM_HIGH_F_MIN;
    	    pwm_max = PWM_HIGH_F_MAX;
    	    // Timer2 shall run 180 deg phase shifted in relation to timer0. Since timer 0
    	    // may run with 7 bit or 8 bit resolution depending on timer clock frequency,
    	    // we use half of the timer 2 TOP value from OCR2A to initialize timer 2.
    	    TCNT2 = (PWM_HIGH_F_TOP >> 1);
    	    break;
    }

    switch (pwm_frequency) {
    	case pwm_frequencyMinimal:
			// TCCR2B – Timer/Counter Control Register B
    		TCCR2B = timer2_ForceOutputCompare_none
				   | timer2_WaveformGenerationModeB_pwmFastOcr
				   | timer2_ClockSelect_div64;
		break;

    	case pwm_frequencyMedium:
    	case pwm_frequencyHigh:
    	default:
    		// TCCR2B – Timer/Counter Control Register B
    		TCCR2B = timer2_ForceOutputCompare_none
				   | timer2_WaveformGenerationModeB_pwmFastOcr
				   | timer2_ClockSelect_1;
		break;
    };
    //restart timer 0
    TCCR0B = timer0_controlregisterB;

    // restart all timers by re-enabling their prescalers
    GTCCR = 0;

    // _shutdown high enables MOSFET-drivers
    PWM_SHTDN_180_PORT |= 1 << PWM_SHTDN_180_BIT;

    //enable interrupts
    sei();
}


