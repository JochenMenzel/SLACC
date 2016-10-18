#include "timer2.h"
#include "time.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>


// Count the time we are already running - might be useful if you want to
// analize data from uart. As there is no difference in computng time between
// big and small (ie. max 1000) values of 16 bit integers multiply the ms-value
// by a factor to increase accuracy on targets with error due to F_CPU.
volatile uint16_t timeMs  = 0; // [ms] * multiplier
volatile uint32_t time    = 0; // [s]


ISR(TIMER2_OVF_vect)
{
    timeMs += TIME_INTERVAL;
    if (timeMs >= 1000 * TIME_MULTIPLIER)
    {
        timeMs -= 1000 * TIME_MULTIPLIER;
        time++;
    }
}


// use timer2 as timebase
void time_init(void)
{
    // TCCR2A – Timer/Counter Control Register A
    TCCR2A = timer2_CompareMatchOutputAMode_normal | timer2_CompareMatchOutputBMode_normal | timer2_WaveformGenerationMode_normal;

    // TCCR2B – Timer/Counter Control Register B
    TCCR2B = timer2_ForceOutputCompare_none | timer2_WaveformGenerationMode_normal;
#if (TIME_TIMER_PRESCALER == 1)
    TCCR2B |= timer2_ClockSelect_1;
#elif (TIME_TIMER_PRESCALER == 8)
    TCCR2B |= timer2_ClockSelect_div8;
#elif (TIME_TIMER_PRESCALER == 32)
    TCCR2B |= timer2_ClockSelect_div32;
#elif (TIME_TIMER_PRESCALER == 64)
    TCCR2B |= timer2_ClockSelect_div64;
#elif (TIME_TIMER_PRESCALER == 128)
    TCCR2B |= timer2_ClockSelect_div128;
#elif (TIME_TIMER_PRESCALER == 256)
    TCCR2B |= timer2_ClockSelect_div256;
#elif (TIME_TIMER_PRESCALER == 1024)
    TCCR2B |= timer2_ClockSelect_div1024;
#endif
 
    // TCNT2 – Timer/Counter Register
    // we do not use a special start value

    // OCR2A – Output Compare Register A
    // unused

    // OCR2B – Output Compare Register B
    // unused

    // TIMSK2 – Timer/Counter2 Interrupt Mask Register
    TIMSK2 = timer2_Interrupt_Overflow;
}

