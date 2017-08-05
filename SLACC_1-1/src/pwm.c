#include "pwm.h"
//#include <avr/io.h>
#include <stdint.h>

uint8_t pwm;

void pwm_init(void)
{
    // configure shutwown as output
    PWM_SHUTDOWN_DDR |= 1 << PWM_SHUTDOWN_BIT;
    // configure pwm pins as output
    DDRD |= 1 << DDD5 | 1 << DDD3; // OC0B, OC2B

    // Timer/Counter2 Interrupt Mask Register
    TIMSK0 = timer0_Interrupt_none;    
    TIMSK2 = timer2_Interrupt_none;

    pwm = PWM_BOTTOM;
    // Output Compare Register B
    OCR0B = OCR2B = pwm;
    // Output Compare Register A
    OCR0A = OCR2A = PWM_TOP;
    
    pwm_disable();
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
    if (pwm >= PWM_MAX)
    {
        pwm = PWM_MAX;
        ret = 1;
    }
    else if (PWM_MAX - pwm >= PWM_STEP)
        pwm += PWM_STEP;
    else
        pwm = PWM_MAX;
        
    OCR0B = OCR2B = pwm;
    
    return ret;
}


// returns 1 if pwm could not be decreased
uint8_t pwm_stepDown(void)
{
    uint8_t ret = 0;
    if (pwm <= PWM_MIN)
    {
        pwm = PWM_MIN;
        ret = 1;
    }
    else if (pwm - PWM_MIN >= PWM_STEP)
        pwm -= PWM_STEP;
    else
        pwm = PWM_MAX;
        
    OCR0B = OCR2B = pwm;
    
    return ret;
}

uint8_t pwm_get(void)
{
    return pwm;
}


void pwm_disable(void)
{
    // _shutdown to low
    PWM_SHUTDOWN_PORT &= ~(1 << PWM_SHUTDOWN_BIT);
    // pwm pins to low
    PORTD &= ~(1 << DDD5 | 1 << DDD3); // OC0B, OC2B

    // set timers to defaults and stop clock
    TCCR0A = TCCR2A = 0;
    TCCR0B = TCCR2B = 0;
}


void pwm_enable(void)
{
    // TCCR0A – Timer/Counter Control Register A
    TCCR0A = timer0_CompareMatchOutputAMode_normal
           | timer0_CompareMatchOutputBMode_clear // use OC0B for unshifted shifted pwm
           | timer0_WaveformGenerationModeA_pwmFastOcr;
    // TCCR2A – Timer/Counter Control Register A
    TCCR2A = timer2_CompareMatchOutputAMode_normal
           | timer2_CompareMatchOutputBMode_clear // use OC2B for 180 deg shifted pwm
           | timer2_WaveformGenerationModeA_pwmFastOcr;
    
    // Set top
    OCR0A = OCR2A = PWM_TOP;
    
    // Set start values to configure phase shift.
    // Using offset of 1 clock per register write to make them all start
    // at the desired values. Tested with with optimization "s" (size), avr-gcc 4.5.3
    
    // Timer/Counter Register
    TCNT0 = 0; // Timer0 - no phase shift
    TCNT2 = (PWM_TOP + 1) >> 1; // Timer1 - 180 deg phase shifted

    // TCCR0B – Timer/Counter Control Register B
    TCCR0B = timer0_ForceOutputCompare_none
           | timer0_WaveformGenerationModeB_pwmFastOcr
           | timer0_ClockSelect_1;
    // TCCR2B – Timer/Counter Control Register B
    TCCR2B = timer2_ForceOutputCompare_none
           | timer2_WaveformGenerationModeB_pwmFastOcr
           | timer2_ClockSelect_1;
 
    // _shutdown high enables MOSFET-drivers
    PWM_SHUTDOWN_PORT |= 1 << PWM_SHUTDOWN_BIT;
}



