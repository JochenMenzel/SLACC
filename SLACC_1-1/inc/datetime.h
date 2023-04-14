// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
// SPDX-FileCopyrightText: 2023 CPRGHT_BM
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _TIME_H__
#define _TIME_H__

#include <stdint.h>
#include <avr/io.h>

/*
Date and Time calculations based on work by Peter Dannegger (danni@specs.de)
See http://www.mikrocontroller.net/topic/25069 .

2012-05-02  Frank Baettermann (frank.baettermann@ich-war-hier.de)
*/


// Use Timer1 (16 bit) to generate timebase
#if (F_CPU == 16000000)
    #define TIME_INTERVAL_MS        2 // [ms]
    #define TIME_TIMER1_OCR1A       (F_CPU / (1000UL / TIME_INTERVAL_MS) - 1)
#else
    #error "I don't know hot to set up Timer1 as timebase."
    #define TIME_INTERVAL_MS    1
    #define TIME_TIMER1_OCR1A   1
#endif


//set the zero value (to 01/01/2000 00:00:00)
#define DATETIME_FIRSTYEAR  2000 // start year
#define DATETIME_FIRSTDAY   6    // 0 = Sunday

typedef struct
{
    uint8_t  second;
    uint8_t  minute;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    uint8_t  wday;
} datetime_t;


void datetime_init(void);
void datetime_set(uint32_t seconds);
uint32_t datetime_getS(void);
uint16_t datetime_getMs(void);
float datetime_getAsFloat(void);
void datetime_timestamp2datetime(uint32_t timestamp, datetime_t *datetime);
char* datetime_nowToS(char* dst);

#endif
