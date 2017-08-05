#include <avr/pgmspace.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "xtoa.h"


// used for unknown values
const char xtoa_None[] PROGMEM = "None";
    
    
// fill string to given length with character
char* strpad(char* dst, char* src, uint8_t length, char padChar, uint8_t right)
{
    uint8_t srcLength = strlen(src);
    uint8_t padLength = (srcLength < length ? length - srcLength : 0);

    if (padLength)
    {
        memset(dst, padChar, length);
        if (right)
            memcpy(dst, src, srcLength); // pad right
        else
            memcpy(&(dst[padLength]), src, srcLength); // pad left
        // add trailing zero
        dst[length] = 0;
    }
    else
        // copy
        strcpy(dst, src);
    
    return dst;
}


// Convert 0 deg Kelvin based temperature to deg Celsius based string.
char* temperatureToA(uint16_t temp, char* dst)
{
    char buffer[4];
    uint16_t left;
    //uint16_t right;
    
    // zero terminator
    dst[0] = 0;
    
    if (temp == UINT16_MAX)
        return strcpy_P(dst, xtoa_None); // no sensor connected/cable broken

    if (temp < TEMPERATURE_ZERO_DEG_C)
    {
        // < 0 °C
        dst[0] = '-';
        dst[1] = 0;
        temp = TEMPERATURE_ZERO_DEG_C - temp;
    }
    else
        temp -= TEMPERATURE_ZERO_DEG_C;
    
    left = temp / 100;
//    right = temp % 100;
    
    utoa(left, buffer, 10);
    strpad(dst,buffer,3,' ',0);
/*    strcat(dst, buffer);
    strcat(dst, ".");
    if (right < 10)
        strcat(dst, "0");
    utoa(right, buffer, 10);
    strcat(dst, buffer);
 */
    return dst;
}


// Convert efficiency to string (1234 --> "12.34")
char* efficiencyToA(uint16_t eff, char* dst)
{
    char buffer[4];
    uint16_t left, right;
    
    // zero terminator
    dst[0] = 0;
    
    if (eff == UINT16_MAX)
        return strcpy_P(dst, xtoa_None); // unknown
    
    left = eff / 100;
    right = eff % 100;
    
    utoa(left, buffer, 10);
    strcat(dst, buffer);
    strcat(dst, ".");
    if (right < 10)
        strcat(dst, "0");
    utoa(right, buffer, 10);
    strcat(dst, buffer);
    
    return dst;
}


