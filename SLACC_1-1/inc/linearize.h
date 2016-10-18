#ifndef _LINEARIZE_H__
#define _LINEARIZE_H__

#include <stdint.h>


/*
Interpolate an output value for a given input value using a calibrated table.

2012-05-01, Frank Bättermann (frank.baettermann@ich-war-hier.de)
*/


typedef struct
{
    uint16_t adc;
    uint16_t value;
} linearizationPointU16_t;


typedef struct
{
    uint8_t size;
    linearizationPointU16_t point[];
} const linearizationTableU16_t;


extern linearizationTableU16_t linListKty81210;
extern linearizationTableU16_t linListBattVoltage;
extern linearizationTableU16_t linListChargeCurrent;
extern linearizationTableU16_t linListPanelCurrent;
extern linearizationTableU16_t linListPanelVoltage;


uint16_t linearizeU16(const linearizationTableU16_t* list, uint16_t adc);

#endif

