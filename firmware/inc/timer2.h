// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _TIME_H__
#define _TIME_H__

#include <stdint.h>
#include <avr/io.h>

#if (F_CPU == 128000)
    // Aim: save power
    // interval 256 ms, error 0%
    #define TIME_TIMER_PRESCALER    64
    #define TIME_MULTIPLIER         1 // max. 32; use power of two to make the compiler shift bits instead of divide/multiply
#elif (F_CPU == 1000000)
    // Aim: save power
    // interval 262 ms, error +0.04%
    #define TIME_TIMER_PRESCALER    1024
    #define TIME_MULTIPLIER         32
#elif (F_CPU == 8000000)
    // Aim: time resolution of 1 ms
    // interval 1 ms, error +0.05%
    #define TIME_TIMER_PRESCALER    32
    #define TIME_MULTIPLIER         32
#elif (F_CPU == 16000000)
    // Aim: time resolution of 1 ms
    // interval 1 ms, error +0.05%
    #define TIME_TIMER_PRESCALER    64
    #define TIME_MULTIPLIER         32
#else
    #error "I don't know hot to set up timer as timebase."
    #define TIME_TIMER_PRESCALER    1
    #define TIME_MULTIPLIER         1
#endif
#define TIME_TIMER_TOP              255
#define TIME_INTERVAL               (1000L * TIME_MULTIPLIER * TIME_TIMER_PRESCALER / (F_CPU / (TIME_TIMER_TOP + 1))) // [ms] * TIME_MULTIPLIER


typedef enum
{
    timer2_CompareMatchOutputAMode_normal   = 0,
    timer2_CompareMatchOutputAMode_toggle   = 1 << COM2A0,
    timer2_CompareMatchOutputAMode_clear    = 1 << COM2A1,
    timer2_CompareMatchOutputAMode_set      = 1 << COM2A1 | 1 << COM2A0
} timer2_CompareMatchOutputAMode_t;

typedef enum
{
    timer2_CompareMatchOutputBMode_normal   = 0,
    timer2_CompareMatchOutputBMode_toggle   = 1 << COM2B0,
    timer2_CompareMatchOutputBMode_clear    = 1 << COM2B1,
    timer2_CompareMatchOutputBMode_set      = 1 << COM2B1 | 1 << COM2B0
} timer2_CompareMatchOutputBMode_t;

typedef enum
{
    timer2_WaveformGenerationMode_normal                = 0,
    timer2_WaveformGenerationModeA_pwmPhaseCorrectMax   = 1 << WGM20,
    timer2_WaveformGenerationModeB_pwmPhaseCorrectMax   = 0,
    timer2_WaveformGenerationModeA_ctc                  = 1 << WGM21,
    timer2_WaveformGenerationModeB_ctc                  = 0,
    timer2_WaveformGenerationModeA_pwmFastMax           = 1 << WGM21 | 1 << WGM20,
    timer2_WaveformGenerationModeB_pwmFastMax           = 0,
    timer2_WaveformGenerationModeA_pwmPhaseCorrectOcr   = 1 << WGM20,
    timer2_WaveformGenerationModeB_pwmPhaseCorrectOcr   = 1 << WGM22,
    timer2_WaveformGenerationModeA_pwmFastOcr           = 1 << WGM21 | 1 << WGM20,
    timer2_WaveformGenerationModeB_pwmFastOcr           = 1 << WGM22
} timer2_WaveformGenerationMode_t;

typedef enum
{
    timer2_ForceOutputCompare_none  = 0,
    timer2_ForceOutputCompare_A     = 1 << FOC2A,
    timer2_ForceOutputCompare_B     = 1 << FOC2B,
} timer2_ForceOutputCompare_t;

typedef enum
{
    timer2_ClockSelect_stopped = 0,
    timer2_ClockSelect_1 = 1 << CS20,
    timer2_ClockSelect_div8 = 1 << CS21,
    timer2_ClockSelect_div32 = 1 << CS21 | 1 << CS20,
    timer2_ClockSelect_div64 = 1 << CS22,
    timer2_ClockSelect_div128 = 1 << CS22 | 1 << CS20,
    timer2_ClockSelect_div256 = 1 << CS22 | 1 << CS21,
    timer2_ClockSelect_div1024 = 1 << CS22 | 1 << CS21 | 1 << CS20
} timer2_ClockSelect_t;

typedef enum
{
    timer2_Interrupt_none           = 0,
    timer2_Interrupt_OutputCompareA = 1 << OCIE2A,
    timer2_Interrupt_OutputCompareB = 1 << OCIE2B,
    timer2_Interrupt_Overflow       = 1 << TOIE2
} timer2_Interrupt_t;


extern volatile uint16_t timeMs;
extern volatile uint32_t time;


void time_init(void);

#endif
