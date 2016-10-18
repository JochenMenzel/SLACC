#include <stdint.h>
#include "linearize.h"


// Temperature
//  o---[4k7]---o---[KTY81-210]---o
// +5V         ADC               GND
linearizationTableU16_t linListKty81210 =
{
    9,
    {
        {1412,  27315UL - 55 * 100}, 
        {1591,  27315UL - 40 * 100},
        {2107,  27315UL}, // 0 °C
        {2645,  27315UL + 40 * 100},
        {3175,  27315UL + 80 * 100},
        {3430,  27315UL + 100 * 100},
        {3719,  27315UL + 125 * 100},
        {3900,  27315UL + 150 * 100},
        {4092,  UINT16_MAX}, // no sensor connected (or connection interrupted)
    }
};


// Battery voltage (2012-04-24, Fluke F175)
//    o---[100k]---o---[18k]---o
// U_Batt         ADC         GND
linearizationTableU16_t linListBattVoltage =
{
    7,
    {
        { 744,  2003}, 
        { 776,  3024},
        {1120,  4512}, // we use a 2,5 reference (values below this have a lower accuracy)
        {2244,  8980},
        {2999, 12000},
        {3503, 14000},
        {4092, 16370},
    }
};


// Battery charge current (2012-04-24, Fluke F175)
//    o------o---[47k]---o
//  INA169  ADC         GND
// TODO: This measurement seems to have an offset of about 40mA.
// Is there anything flowing back into the DCDC-converter?
linearizationTableU16_t linListChargeCurrent =
{
    7,
    { 
        {   0,    45}, // -45mA in reality 
        {  22,   101}, 
        {  60,   200}, 
        { 176,   498}, 
        { 372,  1002}, 
        { 763,  2001}, 
        {4092, 10638}, // untested
    }
};


// Solar panel current (2012-04-24, Fluke F175)
//    o------o---[47k]---o
//  INA169  ADC         GND
linearizationTableU16_t linListPanelCurrent =
{
    6,
    {
        {   0,     0}, 
        {  79,   200}, 
        { 196,   500}, 
        { 388,  1000}, 
        { 776,  2000}, 
        {4092, 10638}, // untested
    }
};


// Solar panel voltage (2012-04-24, Fluke F175)
//    o---[100k]---o---[12k]---o
// U_Batt         ADC         GND
linearizationTableU16_t linListPanelVoltage =
{
    6,
    {
        {   0,     0},
        {1280,  7340},
        {1751, 10000},
        {2632, 15010},
        {3512, 20000},
        {4092, 23230},
    }
};


// Attention: This function does not use absolute 0 and uint16_t maximum. It
// interpolates the value according to the gradient betweeb the two closest points known.
uint16_t linearizeU16(const linearizationTableU16_t* list, uint16_t adc)
{
    uint32_t gradient;
    uint16_t point_adc;
    uint16_t point_value;
   
    // shortcuts for more speed
    point_adc = list->point[0].adc;
    point_value = list->point[0].value;
    if (adc == point_adc)
        return point_value;
    else if (adc < point_adc)
    {
        // multiply by 1024 for accuracy reasons (compiler converts this to fast shift operations)
        gradient = (uint32_t)(list->point[1].value - point_value) * 1024 / (list->point[1].adc - point_adc);
        return (uint16_t)(((uint32_t)point_value * 1024 - gradient * (point_adc - adc)) / 1024);
    }
    
    // if we got here the adc value is greater than the first one in the table
    for (uint8_t p = 1; p < list->size; p++)
    {
        point_adc = list->point[p].adc;
        point_value = list->point[p].value;
        
        if (adc == point_adc)
            return point_value;
        else if (adc < point_adc)
        {
            uint8_t lastp = p - 1;
            gradient = (uint32_t)(point_value - list->point[lastp].value) * 1024 / (point_adc - list->point[lastp].adc);
            return (uint16_t)(((uint32_t)point_value * 1024 - gradient * (point_adc - adc)) / 1024);
        }
    }
    
    // if got here the adc value is greater than any given in the table
    uint8_t i_last = list->size - 1;
    uint8_t i_prev = i_last - 1;
    gradient = (uint32_t)(list->point[i_last].value - list->point[i_prev].value) * 1024 / (list->point[i_last].adc - list->point[i_prev].adc);
    return (uint16_t)(((uint32_t)list->point[i_last].value * 1024 + gradient * (adc - list->point[i_last].adc)) / 1024);
}

