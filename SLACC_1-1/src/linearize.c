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
        {1120,  4412}, // we use a 2,5 reference (values below this have a lower accuracy)
        {2244,  8800},
        {2999, 11730},
        {3503, 13690},
        {4092, 15960},
    }
};


// Battery charge current (2017-08-03, 6mOhm shunt)
//    o------o---[200k]---o
//  INA169  ADC         GND

linearizationTableU16_t linListChargeCurrent =
{
	6,
	{
			{0,0},
			{512,1302},
			{1024,2604},
			{2048,5208},
			{3072,7813},
			{4092,10406},
	}
};


// Solar panel current (2017-08-03, 16mOhm shunt)
//    o------o---[180k]---o
//  INA169  ADC         GND
linearizationTableU16_t linListPanelCurrent =
{
	6,
	{
			{0,0},
			{512,574},
			{1024,1170},
			{2048,2370},
			{3072,3255},
			{4092,4336},
	}
};


// Solar panel voltage (2017-08-02)
//    o---[100k]---o---[5k6]---o
// U_Batt         ADC         GND
linearizationTableU16_t linListPanelVoltage =
{
    6,
    {
        {   0,     0},
        {1280, 14732},
        {1751, 20153},
        {2632, 30293},
        {3512, 40421},
        {4092, 47097},
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

