#include <adc.h>
#include <stdint.h>

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
    /*
     * ADCSRA – ADC Control and Status Register A.
     * adc.h automatically calculates prescaler settings for maximum allowable ADC clock.
     */
    ADCSRA = autoTrigger | interrupt | ADC_PRESCALER_REG;
    // ADCSRB – ADC Control and Status Register B
    ADCSRB = autoTriggerSource;
    // disable digital input buffers for the six double-usage pins - we use all of them as analog inputs.
    DIDR0 = (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D) | (1 << ADC3D) | (1 << ADC5D) | (1 << ADC5D) ;
}


// single conversion of selected channel
// With ADC_clock = 16 MHz / 128 = 125kHz and 13 ADC_clock cycles per conversion, this takes 1.04ms.
uint16_t adc_singleConversion(void)
{
    ADCSRA |= (1 << ADSC); // start conversion
    while (ADCSRA & (1 << ADSC)); // loop until the conversion is complete
    return ADCW;
}


// 16x supersampling to achieve 12 bit resolution with 10 bit ADC
// since each sample needs roughly 1.04 ms, this takes approx.  16,64 ms.
uint16_t adc_12BitConversion(uint8_t channel)
{
    uint16_t adc = 0;
    adc_setChannel(channel);
    for (uint8_t i = 0; i < 16; i++)
        adc += adc_singleConversion();
    return adc >> 2; // divide by 4
}

/*
 * disable power to analog comparator
 */
void analog_comparator_disable(void){
	//disable power to analog comparator in Analog Comparator Control and Status Register
	ACSR = (1<<ACD);
}
