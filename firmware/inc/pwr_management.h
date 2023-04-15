// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * pwr_management.h
 *
 *  Created on: 17.12.2018
 *      Author: dermeisterr
 */

#ifndef INC_PWR_MANAGEMENT_H_
#define INC_PWR_MANAGEMENT_H_

/* define GPIO port that controls power to the PTCs and the external ADC reference voltage source */
#define PTC_ADCREF_PORT      PORTB
#define PTC_ADCREF_DDR       DDRB
#define PTC_ADCREF           DDB0


void PTC_ADCref_init(void);
void PTC_ADCref_on(void);
void PTC_ADRref_off(void);

/*
 * activate watchdog, shutdown all other stuff and go to sleep
 */
void goToSleep (void);

/*
 * switch off power to hardware i2c interface.
 */
void power_twi_spi_usart_disable(void);

#endif /* INC_PWR_MANAGEMENT_H_ */
