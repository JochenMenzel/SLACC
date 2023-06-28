// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "datetime.h"


// Local counters; access them 
volatile uint16_t datetime_ms = 0;  // [milliseconds] 
volatile uint32_t datetime_s = 0;   // [seconds]


// for AVR FAT32
volatile uint8_t TimingDelay_1mscount = 0;  // incremented every ms, to produce 10ms interval for FAT32
volatile uint8_t TimingDelay = 0;           // decrement every 10ms (if >0)


ISR(TIMER1_COMPA_vect)
{
    datetime_ms += TIME_INTERVAL_MS;
    if (datetime_ms >= 1000)
    {
        datetime_ms -= 1000;
        datetime_s++;
    }
    
    // for AVR FAT32
    if (++TimingDelay_1mscount == 10)
    {
        if (TimingDelay)
            TimingDelay--;
    }
}


// Initialize Timer and timestamp
void datetime_init(void)
{
    // Timer1 as timebase
    TCCR1A = 0; // not output, not pwm
    TCCR1B = 1 << WGM12 | 1 << CS10; // clear timer on compare (OCR1A), no prescaler
    OCR1A = TIME_TIMER1_OCR1A;
    TCNT1 = 0; // start with 0
    TIMSK1 = 1 << OCIE1A; // enable output compare 1a interrupt
    
    // initial date
    datetime_set(0);
}


// Set timestamp
void datetime_set(uint32_t seconds)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        datetime_s = seconds;
        datetime_ms = 0;
    }
}


// Return current timestamp
uint32_t datetime_getS(void)
{
    uint32_t s;
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        s = datetime_s;
    }
    return s;
}


// Returns current milliseconds
uint16_t datetime_getMs(void)
{
    uint16_t ms;
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        ms = datetime_ms;
    }
    return ms;
}


// Returns time as float
float datetime_getAsFloat(void)
{
    uint32_t s, ms;
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        s = datetime_s;
        ms = datetime_ms;
    }
    return (float)s + (float)ms / 1000;
}


// Convert timestamp to datetime-struct
uint8_t datetime_DayOfMonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void datetime_timestamp2datetime(uint32_t timestamp, datetime_t *datetime)
{
    uint16_t day;
    uint8_t year;
    uint16_t dayofyear;
    uint8_t leap400;
    uint8_t month;

    // time
    datetime->second = timestamp % 60;
    timestamp /= 60;
    datetime->minute = timestamp % 60;
    timestamp /= 60;
    datetime->hour = timestamp % 24;
    day = timestamp / 24;

    // weekday
    datetime->wday = (day + DATETIME_FIRSTDAY) % 7;

    year = DATETIME_FIRSTYEAR % 100;                    // 0..99
    leap400 = 4 - ((DATETIME_FIRSTYEAR - 1) / 100 & 3); // 4, 3, 2, 1

    while (1)
    {
        dayofyear = 365;
        if ((year & 3) == 0)
        {
            // leap year
            dayofyear = 366;
            if (year == 0 || year == 100 || year == 200) // 100 year exception
                if (--leap400) // 400 year exception
                    dayofyear = 365;
        }
        if (day < dayofyear)
            break;
        day -= dayofyear;
        year++; // 00..136 / 99..235
    }

    datetime->year = year + DATETIME_FIRSTYEAR / 100 * 100; // + century

    if (dayofyear & 1 && day > 58)  // no leap year and after 28.2.
        day++;                      // skip 29.2.

    for (month = 1; day >= datetime_DayOfMonth[month - 1]; month++)
        day -= datetime_DayOfMonth[month - 1];

    datetime->month = month; // 1..12
    datetime->day = day + 1; // 1..31
}


// Convert timestamp to string on ms base (max "4294967295.999\0", therefore
// 15 bytes of output buffer needed)
char* datetime_nowToS(char* dst)
{
    // local buffer for milliseconds
    char buffer[4];
    
    // create local copy of datetime
    uint32_t s;
    uint16_t ms;
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        s = datetime_s;
        ms = datetime_ms;
    }

    // whole seconds
    ultoa(s, dst, 10);
    strcat_P(dst, PSTR("."));

    // leading zeroes for milliseconds
    if (ms < 10)
        strcat_P(dst, PSTR("00"));
    else if (ms < 100)
        strcat_P(dst, PSTR("0"));
    utoa(ms, buffer, 10);
    strcat(dst, buffer);
    
    return dst;
}


