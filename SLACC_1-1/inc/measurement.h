#ifndef _MEASUREMENT_H__
#define _MEASUREMENT_H__

#include <stdint.h>


// Use these variables the calculate averages.
#define ADCAVG_SAMPLES 4   // the use of power of two turns division into fast shift operation
#define ADCAVG_MAX     4092 // supersampled 12 bit values
#if (ADCAVG_SAMPLES > UINT8_MAX)
    #error "ADC averaging: Numer of samples is too high."
#endif

#if (ADCAVG_SAMPLES * ADCAVG_MAX > UINT32_MAX)
    #error "ADC averaging: uint32 cannot hold samples*max."
    #define ADCAVG_SUM_TYPE uint32_t
#elif (ADCAVG_SAMPLES * ADCAVG_MAX > UINT16_MAX)
    #define ADCAVG_SUM_TYPE uint32_t
#else 
    #define ADCAVG_SUM_TYPE uint16_t
#endif


typedef struct
{
    uint16_t adc;
    uint16_t v; // value
} measurement16_t;


typedef struct
{
    measurement16_t temperature1;   // [°K * 100]
    measurement16_t temperature2;   // [°K * 100]
    measurement16_t temperature3;   // [°K * 100]
    measurement16_t panelVoltage;   // [mV]
    measurement16_t panelCurrent;   // [mA]
    uint16_t panelPower;            // [W * 100]
    measurement16_t batteryVoltage; // [mV]
    measurement16_t chargeCurrent;  // [mA]
    uint16_t chargePower;           // [W * 100]
    uint16_t efficiency;            // [% * 100]
} measurements_t;


extern measurements_t measurements;

void measure(void);

#endif

