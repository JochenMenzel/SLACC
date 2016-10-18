#include <stdint.h>
#include <avr/io.h>
#include "led.h"
#include "main.h"
#include "measurement.h"


// current voltage LED status
led_voltage_t ledVoltage;


void led_init(void)
{
    // set pins as output
    LED_CHARGE_DDR |= 1 << LED_CHARGE;
    LED_U_RED_DDR |= 1 << LED_U_RED;
    LED_U_GREEN_DDR |= 1 << LED_U_GREEN;
    
    // internal status
    ledVoltage = led_voltageOff;
}


void led_update(void)
{
    // local copy of battery voltage
    uint16_t uBatt = measurements.batteryVoltage.v;
    
    // choose new battery voltage status
    switch (ledVoltage)
    {
        case led_voltageHigh:
            if (uBatt < LED_VOLTAGE_HIGH - LED_VOLTAGE_HYSTERESIS / 2) // high --> medium
                ledVoltage = led_voltageMedium;
            break;
        
        case led_voltageMedium:
            if (uBatt > LED_VOLTAGE_HIGH + LED_VOLTAGE_HYSTERESIS / 2) // medium --> high
                ledVoltage = led_voltageHigh;
            else if (uBatt < LED_VOLTAGE_MEDIUM - LED_VOLTAGE_HYSTERESIS / 2) // medium --> low
                ledVoltage = led_voltageLow;
        
        case led_voltageLow:
            if (uBatt > LED_VOLTAGE_MEDIUM + LED_VOLTAGE_HYSTERESIS / 2) // low --> medium
                ledVoltage = led_voltageMedium;
        
        case led_voltageOff: // fall through
        default:
            ledVoltage = led_voltageLow; // default to low by now
    }
    
    // set LED ports to new status
    switch (ledVoltage)
    {
        case led_voltageLow:
            // red
            LED_U_GREEN_PORT &= ~(1 << LED_U_GREEN);
            LED_U_RED_PORT |= 1 << LED_U_RED;
            break;
            
        case led_voltageMedium:
            // yellow
            LED_U_RED_PORT |= 1 << LED_U_RED;
            LED_U_GREEN_PORT |= 1 << LED_U_GREEN;
            break;
            
        case led_voltageHigh:
            // green
            LED_U_RED_PORT &= ~(1 << LED_U_RED);
            LED_U_GREEN_PORT |= 1 << LED_U_GREEN;
            break;
            
        case led_voltageOff: // fall through
        default:
            // off
            LED_U_RED_PORT &= ~(1 << LED_U_RED);
            LED_U_GREEN_PORT &= ~(1 << LED_U_GREEN);
    }

    // charge LED
    if (chargerStatus & chargerStatus_charging)
        // on
        LED_CHARGE_PORT |= 1 << LED_CHARGE;
    else
        // off
        LED_CHARGE_PORT &= ~(1 << LED_CHARGE);
}

