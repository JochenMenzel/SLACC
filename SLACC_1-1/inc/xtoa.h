#ifndef _XTOA_H__
#define _XTOA_H__

#include <stdint.h>

#define TEMPERATURE_ZERO_DEG_C 27315 // [K*100]

char* strpad(char* dst, char* src, uint8_t length, char padChar, uint8_t right);
char* temperatureToA(uint16_t temp, char* dst);
char* efficiencyToA(uint16_t eff, char* dst);

#endif
