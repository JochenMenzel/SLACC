// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
