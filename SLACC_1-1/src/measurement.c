#include <stdint.h>
#include "adc.h"
#include "linearize.h"
#include "measurement.h"


// struct that saves all measurements
measurements_t measurements;


void measure(void)
{
    ADCAVG_SUM_TYPE adcSum;
    uint32_t PTCcorrection;

    // ADC0 = temperature 3
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(0);
    measurements.PTCsupply.adc = adcSum / ADCAVG_SAMPLES;
//    measurements.PTCsupply.v = 5000 * measurements.PTCsupply.adc / 4096;

    // ADC1 = panel current
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(1);
    measurements.panelCurrent.adc = adcSum / ADCAVG_SAMPLES;
    measurements.panelCurrent.v = linearizeU16(&linListPanelCurrent, measurements.panelCurrent.adc);

    // ADC2 = panel voltage
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(2);
    measurements.panelVoltage.adc= adcSum / ADCAVG_SAMPLES;
    measurements.panelVoltage.v = linearizeU16(&linListPanelVoltage, measurements.panelVoltage.adc);
   
    // ADC4 = battery voltage
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(4);
    measurements.batteryVoltage.adc = adcSum / ADCAVG_SAMPLES;
    measurements.batteryVoltage.v = linearizeU16(&linListBattVoltage, measurements.batteryVoltage.adc);
    
    // ADC5 = charge current
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(5);
    measurements.chargeCurrent.adc = adcSum / ADCAVG_SAMPLES;
    measurements.chargeCurrent.v = linearizeU16(&linListChargeCurrent, measurements.chargeCurrent.adc);

    // ADC6 = temperature 1
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(6);
    measurements.temperature1.adc = adcSum / ADCAVG_SAMPLES;
    //correct for PTC halfbridge supply voltage that is unequal to 5.00 V
    PTCcorrection = (long) measurements.temperature1.adc * (long)4095 / (long) measurements.PTCsupply.adc;
    measurements.temperature1.adc = (uint16_t) PTCcorrection ;
    measurements.temperature1.v = linearizeU16(&linListKty81210, measurements.temperature1.adc);
    
    // ADC7 = temperature 2
    adcSum = 0;
    for (uint8_t i = 0; i < ADCAVG_SAMPLES; i++)
        adcSum += adc_12BitConversion(7);
    measurements.temperature2.adc = adcSum / ADCAVG_SAMPLES;
    PTCcorrection = (long) measurements.temperature2.adc * (long)4095 / (long) measurements.PTCsupply.adc;
    measurements.temperature2.adc = (uint16_t) PTCcorrection ;
    measurements.temperature2.v = linearizeU16(&linListKty81210, measurements.temperature2.adc);
    
    // Compute values
    uint32_t chargePowerPrecise = (uint32_t)measurements.batteryVoltage.v * (uint32_t)measurements.chargeCurrent.v;
    measurements.panelPower = (uint16_t)((uint32_t)measurements.panelVoltage.v * (uint32_t)measurements.panelCurrent.v / 10000UL);
    measurements.chargePower = (uint16_t)(chargePowerPrecise / 10000UL);
    
    if (measurements.panelPower > measurements.chargePower)
        measurements.efficiency = (uint16_t)(chargePowerPrecise / measurements.panelPower);
    else
        measurements.efficiency = UINT16_MAX; // more than 100% efficiency would not make sense
}

