#include <stdint.h>
#include <avr/io.h>
#include "fan.h"
#include "main.h"
#include "measurement.h"

void fan_init(void)
{
	// set pins as output
    FAN_DDR |= 1 << FAN_BIT; // connect load
}

void fan_on(void){
    // on
    FAN_PORT |= 1 << FAN_BIT;

}

void fan_off(void){
    // off
    FAN_PORT &= ~(1 << FAN_BIT);
}
