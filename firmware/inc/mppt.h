// SPDX-FileCopyrightText: 2023 2019 Dr. Martin Jäger (https://libre.solar)
//
// SPDX-License-Identifier: Apache-2.0

/*
 * mppt.h
 *
 *  Created on: 17.12.2018
 *      Author: dermeisterr
 */

#ifndef INC_MPPT_H_
#define INC_MPPT_H_

#include "measurement.h"
#include "charger.h"

void update_mppt(measurements_t * measurements, ChargingProfile * profile);

#endif /* INC_MPPT_H_ */