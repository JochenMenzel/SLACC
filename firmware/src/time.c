// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "time.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>


// Count the time we are already running
volatile uint16_t timeMs  = 0; // [ms] 
volatile uint32_t time    = 0; // [s]

// for AVR FAT32
volatile uint8_t TimingDelay_1mscount = 0; // increment every ms, to produce 10ms interval for FAT32
volatile uint8_t TimingDelay = 0; // decrement every 10ms (if >0)


ISR(TIMER1_COMPA_vect)
{
    timeMs += TIME_INTERVAL_MS;
    if (timeMs >= 1000)
    {
        timeMs -= 1000;
        time++;
    }
    
    if (++TimingDelay_1mscount == 10)
    {
        if (TimingDelay)
            TimingDelay--;
    }
}


// Timer1 as timebase
void time_init(void)
{
    TCCR1A = 0; // not output, not pwm
    TCCR1B = 1 << WGM12 | 1 << CS10; // clear timer on compare (OCR1A), no prescaler
    OCR1A = TIME_TIMER1_OCR1A;
    TCNT1 = 0; // start with 0
    TIMSK1 = 1 << OCIE1A; // enable output compare 1a interrupt
}

