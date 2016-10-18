#ifndef _ADC_H__
#define _ADC_H__

#include <stdint.h>
#include <avr/io.h>

// according to manual the adc-clock should be below 200 kHz
#define ADC_CLOCK_MAX 200000

// select prescaler for maximum adc-frequency
#if (F_CPU < ADC_CLOCK_MAX)
    #define ADC_PRESCALER       1
    #define ADC_PRESCALER_REG   0
#elif ((F_CPU / 2) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       2
    #define ADC_PRESCALER_REG   (1 << ADPS0);
    #define ADC_CLOCK           (F_CPU / 2)
#elif ((F_CPU / 4) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       4
    #define ADC_PRESCALER_REG   (1 << ADPS1)
#elif ((F_CPU / 8) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       8
    #define ADC_PRESCALER_REG   (1 << ADPS1 | 1 << ADPS0)
#elif ((F_CPU / 16) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       16
    #define ADC_PRESCALER_REG   (1 << ADPS2)
#elif ((F_CPU / 32) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       32
    #define ADC_PRESCALER_REG   (1 << ADPS2 | 1 << ADPS0)
#elif ((F_CPU / 64) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       64
    #define ADC_PRESCALER_REG   (1 << ADPS2 | 1 << ADPS1)
#elif ((F_CPU / 128) <= ADC_CLOCK_MAX)
    #define ADC_PRESCALER       128
    #define ADC_PRESCALER_REG   (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0)
#else
    #error "Reduce F_CPU to keep the ADC-clock below 200kHz."
#endif
#define ADC_CLOCK               (F_CPU / ADC_PRESCALER)
#define ADC_REFVOLTAGE_AREF     0 // [mV] external reference voltage
#define ADC_REFVOLTAGE_INT      1100 // [mV]
#define ADC_REFVOLTAGE_AVCC     5000 // [mV]


typedef enum
{
    adc_voltageReferenceAref        = 0,
    adc_voltageReferenceAvcc        = 1 << REFS0,
    adc_voltageReferenceInternal    = 1 << REFS1 | 1 << REFS0
} adc_voltageReference;


typedef enum
{
    adc_adjustResultRight   = 0,
    adc_adjustResultLeft    = 1 << ADLAR
} adc_adjustResult;


typedef enum
{
    adc_interruptDisabled   = 0,
    adc_interruptEnabled    = 1 << ADIE
} adc_interrupt;


typedef enum
{
    adc_autoTriggerDisabled   = 0,
    adc_autoTriggerEnabled    = 1 << ADATE
} adc_autoTrigger;


typedef enum
{
    adc_autoTriggerSourceFreeRunning        = 0,
    adc_autoTriggerSourceAnalogComp         = 1 << ADTS0,
    adc_autoTriggerSourceExternalIntReq0    = 1 << ADTS1,
    adc_autoTriggerSourceTimer0CompareA     = 1 << ADTS1 | 1 << ADTS0,
    adc_autoTriggerSourceTimer0Overflow     = 1 << ADTS2,
    adc_autoTriggerSourceTimer1CompareB     = 1 << ADTS2 | 1 << ADTS0,
    adc_autoTriggerSourceTimer1Overflow     = 1 << ADTS2 | 1 << ADTS1,
    adc_autoTriggerSourceTimer1CaptureEvent = 1 << ADTS2 | 1 << ADTS1 | 1 << ADTS0
} adc_autoTriggerSource;


void adc_init(adc_voltageReference, adc_adjustResult, adc_interrupt, adc_autoTrigger, adc_autoTriggerSource);


static inline void adc_setChannel(uint8_t channel)
{
    // Map
    // -----------------------------------------
    // 0-7:     singel ended channels
    // 8:       internal temperature reference
    // 9-13:    reserved
    // 14:      internal voltage reference
    // 15:      gnd
    
    // As the MUX3:0-bits are right aligned in ADMUX-register we only mask them
    ADMUX = (ADMUX & 0xf0) | (channel & 0x0f);
};


static inline void adc_enable(void)
{
    ADCSRA |= (1 << ADEN);
};


static inline void adc_disable(void)
{
    ADCSRA &= ~(1 << ADEN);
};


uint16_t adc_singleConversion(void);
uint16_t adc_12BitConversion(uint8_t channel);

#endif

