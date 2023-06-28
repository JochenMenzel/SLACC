// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _TIME_H__
#define _TIME_H__

#include <stdint.h>
#include <avr/io.h>

// Use Timer1 (16 bit) to generate timebase
#if (F_CPU == 16000000)
    #define TIME_INTERVAL_MS        2 // [ms]
    #define TIME_TIMER1_OCR1A       (F_CPU / (1000UL / TIME_INTERVAL_MS) - 1)
#else
    #error "I don't know hot to set up Timer1 as timebase."
    #define TIME_INTERVAL_MS    1
    #define TIME_TIMER1_OCR1A   1
#endif


extern volatile uint16_t timeMs;
extern volatile uint32_t time;

void time_init(void);

#endif
