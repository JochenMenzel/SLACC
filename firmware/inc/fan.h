// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _LED_H__
#define _LED_H__

#include <stdint.h>
#include <avr/io.h>

#define FAN_PORT   PORTD
#define FAN_DDR    DDRD
#define FAN_BIT    DDD2
//#PD2 used to be Load control in original design by F. Bättermann. Changed to fan control by JMe in `17

// LED hardware connections
#define LED_CHARGE_PORT     PORTD
#define LED_CHARGE_DDR      DDRD
#define LED_CHARGE          DDD6

#define LED_U_GREEN_PORT    PORTD
#define LED_U_GREEN_DDR     DDRD
#define LED_U_GREEN         DDD7


void fan_init(void);

//turn on / off fan connected via GPIO
void fan_on(void);
void fan_off(void);
#endif

