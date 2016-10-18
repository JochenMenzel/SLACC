#include "adc.h"
#include <stdint.h>
#include <avr/io.h>

/*
Use the analog/digital-converter a more comfortable way.

Missing features: input-gain, auto trigger enable

Target:      ATMega48, ATMega88, ATMega168, ATMega328
Last Change: 2012-04-24 (Frank Baettermann)
*/

void adc_init(adc_voltageReference voltageReference, adc_adjustResult adjustResult, adc_interrupt interrupt, adc_autoTrigger autoTrigger, adc_autoTriggerSource autoTriggerSource)
{
    // ADMUX – ADC Multiplexer Selection Register
    ADMUX = voltageReference | adjustResult;
    // ADCSRA – ADC Control and Status Register A
    ADCSRA = autoTrigger | interrupt | ADC_PRESCALER_REG;
    // ADCSRB – ADC Control and Status Register B
    ADCSRB = autoTriggerSource;
}


// single conversion of selected channel
uint16_t adc_singleConversion(void)
{
    ADCSRA |= (1 << ADSC); // start conversion
    while (ADCSRA & (1 << ADSC)); // loop until the conversion is complete
    return ADCW;
}


// 16x supersampling to achieve 12 bit resolution with 10 bit ADC
uint16_t adc_12BitConversion(uint8_t channel)
{
    uint16_t adc = 0;
    adc_setChannel(channel);
    for (uint8_t i = 0; i < 16; i++)
        adc += adc_singleConversion();
    return adc >> 2; // divide by 4
}

