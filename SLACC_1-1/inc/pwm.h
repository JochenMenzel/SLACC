#ifndef _PWM_H__
#define _PWM_H__

#include <stdint.h>
#include <avr/io.h>


/*
Use Timer0 and Timer2 (both 8 Bit) for phase shifted pwm (16 MHz / 256 = 62.5 kHz).
*/


#if (F_CPU != 16000000)
    #error "I don't know hot to set up pmw-timers."
#endif

//define register access for shutdown signal of the 0� buck stage
#define PWM_SHTDN_0_PORT       PORTC
#define PWM_SHTDN_0_DDR        DDRC
#define PWM_SHTDN_0_BIT        DDC3

//define register access for shutdown signal of the 180� buck stage
#define PWM_SHTDN_180_PORT       PORTB
#define PWM_SHTDN_180_DDR        DDRB
#define PWM_SHTDN_180_BIT        DDB1

// We may never set pwm to 100% because the MosFet bootstrap wouldn't work anymore
// 970 Hz / 62500 Hz
#define PWM_MEDIUM_F_TOP            255
#define PWM_MEDIUM_F_BOTTOM         0
#define PWM_MEDIUM_F_STEP           2
#define PWM_MEDIUM_F_MAX            (PWM_MEDIUM_F_TOP - PWM_MEDIUM_F_STEP)    // maximum value one should set to keep boostrap working
#define PWM_MEDIUM_F_MIN            (PWM_MEDIUM_F_TOP * 25UL / 100) // 25% is minimum
#define PWM_MEDIUM_F_INIT_OFFSET    (PWM_MEDIUM_F_TOP * 4UL / 100) // add 4% pwm value when guessing first pwm values
// 125000 Hz
#define PWM_HIGH_F_TOP              127
#define PWM_HIGH_F_BOTTOM           0
#define PWM_HIGH_F_STEP             1
#define PWM_HIGH_F_MAX              (PWM_HIGH_F_TOP - PWM_HIGH_F_STEP)
#define PWM_HIGH_F_MIN              (PWM_HIGH_F_TOP * 25UL / 100)
#define PWM_HIGH_F_INIT_OFFSET      (PWM_HIGH_F_TOP * 4UL / 100)


// TODO: On the fly switching not implemented, yet!
#define PWM_TOP                     PWM_HIGH_F_TOP
#define PWM_BOTTOM                  PWM_HIGH_F_BOTTOM
#define PWM_STEP                    PWM_HIGH_F_STEP
#define PWM_MAX                     PWM_HIGH_F_MAX
#define PWM_MIN                     PWM_HIGH_F_MIN
#define PWM_INIT_OFFSET             PWM_HIGH_F_INIT_OFFSET

typedef enum
{
    pwm_frequencyMinimal = 0, // ~1 kHz, pwm set to max
    pwm_frequencyMedium  = 1, // 62.5 kHz, 8 bit pwm
    pwm_frequencyHigh    = 2  // 125 kHz, 7 bit pwm
} pwm_frequency_t;


// Timer0s

typedef enum
{
    timer0_CompareMatchOutputAMode_normal   = 0,
    timer0_CompareMatchOutputAMode_toggle   = 1 << COM0A0,
    timer0_CompareMatchOutputAMode_clear    = 1 << COM0A1,
    timer0_CompareMatchOutputAMode_set      = 1 << COM0A1 | 1 << COM0A0
} timer0_CompareMatchOutputAMode_t;

typedef enum
{
    timer0_CompareMatchOutputBMode_normal   = 0,
    timer0_CompareMatchOutputBMode_toggle   = 1 << COM0B0,
    timer0_CompareMatchOutputBMode_clear    = 1 << COM0B1,
    timer0_CompareMatchOutputBMode_set      = 1 << COM0B1 | 1 << COM0B0
} timer0_CompareMatchOutputBMode_t;

typedef enum
{
    timer0_WaveformGenerationMode_normal                = 0,
    timer0_WaveformGenerationModeA_pwmPhaseCorrectMax   = 1 << WGM00,
    timer0_WaveformGenerationModeB_pwmPhaseCorrectMax   = 0,
    timer0_WaveformGenerationModeA_ctc                  = 1 << WGM01,
    timer0_WaveformGenerationModeB_ctc                  = 0,
    timer0_WaveformGenerationModeA_pwmFastMax           = 1 << WGM01 | 1 << WGM00,
    timer0_WaveformGenerationModeB_pwmFastMax           = 0,
    timer0_WaveformGenerationModeA_pwmPhaseCorrectOcr   = 1 << WGM00,
    timer0_WaveformGenerationModeB_pwmPhaseCorrectOcr   = 1 << WGM02,
    timer0_WaveformGenerationModeA_pwmFastOcr           = 1 << WGM01 | 1 << WGM00,
    timer0_WaveformGenerationModeB_pwmFastOcr           = 1 << WGM02
} timer0_WaveformGenerationMode_t;

typedef enum
{
    timer0_ForceOutputCompare_none  = 0,
    timer0_ForceOutputCompare_A     = 1 << FOC0A,
    timer0_ForceOutputCompare_B     = 1 << FOC0B,
} timer0_ForceOutputCompare_t;

typedef enum
{
    timer0_ClockSelect_stopped = 0,
    timer0_ClockSelect_1 = 1 << CS00,
    timer0_ClockSelect_div8 = 1 << CS01,
    timer0_ClockSelect_div32 = 1 << CS01 | 1 << CS00,
    timer0_ClockSelect_div64 = 1 << CS02,
    timer0_ClockSelect_div128 = 1 << CS02 | 1 << CS00,
    timer0_ClockSelect_div256 = 1 << CS02 | 1 << CS01,
    timer0_ClockSelect_div1024 = 1 << CS02 | 1 << CS01 | 1 << CS00
} timer0_ClockSelect_t;

typedef enum
{
    timer0_Interrupt_none           = 0,
    timer0_Interrupt_OutputCompareA = 1 << OCIE0A,
    timer0_Interrupt_OutputCompareB = 1 << OCIE0B,
    timer0_Interrupt_Overflow       = 1 << TOIE0
} timer0_Interrupt_t;


// Timer2

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


void pwm_init(void);
void pwm_set(uint8_t pwm);
uint8_t pwm_stepDown(void);
uint8_t pwm_stepUp(void);
uint8_t pwm_get(void);
void pwm_0deg_disable(void);
void pwm_180deg_disable(void);
void pwm_0deg_enable(pwm_frequency_t pwm_frequency);
void pwm_180deg_enable(pwm_frequency_t pwm_frequency);
#endif

