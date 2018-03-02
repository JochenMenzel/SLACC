#ifndef _LED_H__
#define _LED_H__

#include <stdint.h>
#include <avr/io.h>

// LED hardware connections
#define LED_CHARGE_PORT     PORTD
#define LED_CHARGE_DDR      DDRD
#define LED_CHARGE          DDD6
#define LED_U_RED_PORT      PORTB
#define LED_U_RED_DDR       DDRB
#define LED_U_RED           PB0
#define LED_U_GREEN_PORT    PORTD
#define LED_U_GREEN_DDR     DDRD
#define LED_U_GREEN         DDD7

// If the battery voltage is above these values choose the apropriate led-color
#define LED_VOLTAGE_HIGH            12200 // [mV]
#define LED_VOLTAGE_MEDIUM          11900 // [mV]
#define LED_VOLTAGE_HYSTERESIS      50 // [mV]


typedef enum
{
    led_voltageOff,
    led_voltageLow,
    led_voltageMedium,
    led_voltageHigh
} led_voltage_t;


void led_init(void);
void led_update(void);


#endif

