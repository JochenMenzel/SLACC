/*
 * hmi.h
 *
 *  Created on: 16.02.2018
 *      Author: dermeisterr
 */

#ifndef INC_HMI_H_
#define INC_HMI_H_

/*
 * shows panel voltage, panel current, battery voltage, battery current, temperature and operating state on
 * a 16 x 2 character LCD display.
 */
void showProcessValues(measurements_t measurements);

/*
 * shows panel voltage, battery voltage and the time in sleep mode on a 16 x 2 character LCD display.
 */
void showSleepMessage(measurements_t measurements);

#endif /* INC_HMI_H_ */
