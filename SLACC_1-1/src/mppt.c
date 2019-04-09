/*
 * mppt.c
 *
 * LibreSolar MPPT charge controller mppt algorithm
 * Copyright (c) 2016-2018 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mppt.h"
#include "measurement.h"
#include "charger.h"
#include "pwm.h"
#include "datetime.h"

uint32_t dcdc_power;    // stores previous output power
uint8_t MPPT_direction_up = 0xFF;

void update_mppt(measurements_t * measurements, ChargingProfile * profile)
{
    uint32_t dcdc_power_new = measurements->panelPower;
    //uint32_t dcdc_power_new = measurements->chargePower;

//    if (dcdc_enabled() == false && charger_charging_enabled() == true
    if (!isCharging()
        && measurements->batteryVoltage.v < charger_read_target_voltage()
 //       && measurements->batteryVoltage.v > profile->battery_voltage_absolute_min
        && (measurements->panelVoltage.v > measurements->batteryVoltage.v)
        && (datetime_getS() > profile->restart_charging_time))
    {
//        serial.printf("MPPT start!\n");
    	startCharging();
    }
    else if (isCharging() &&
 //   		(measurements->panelVoltage.v <= measurements->batteryVoltage.v) &&
			(measurements->panelCurrent.v < profile->charge_panel_current_min))
    {
        //serial.printf("MPPT stop!\n");
        stopCharging();

    }
    else if (isCharging()) {

        if (measurements->batteryVoltage.v > charger_read_target_voltage()
            || measurements->chargeCurrent.v > charger_read_target_current()
            || isOvertemperature1()
			|| isOvertemperature2())
        {
            // increase input voltage --> lower output voltage and decreased current
			pwm_stepDown();
        }
        else if (measurements->panelCurrent.v < profile->mppt_panel_current_min) {
            // do not make use of MPPT because there seems to be little sun and we have veery low panel current.

        	/*
			 * if pwm < PWM_TOP * battery voltage / panel voltage, we need to correct the duty cycle or we
			 * will see current flow from the battery through the charger into the panel.
			 * Check if charge current is approximately same as panel current. If not, increase PWM duty cycle.
			 * We compare panel current against charge current + 10 mA because an active SLACC consumes 10 mA
			 * for its own circuitry.
			 */
        	if(measurements->panelCurrent.v > measurements->chargeCurrent.v + 10){
        		/*
        		 * additionally increase pwm duty cycle
        		 */
        	    // Guess initial pwm setting: pwm = batteryVoltage * PWM_TOP / panelVoltage + offset
        	    pwm = (uint16_t)(((uint32_t)measurements->batteryVoltage.v * PWM_TOP) / measurements->panelVoltage.v) + PWM_INIT_OFFSET;
        	    if (pwm > PWM_MAX-4)
        	    	pwm = PWM_MAX-4;
        	}

        	//let PWM duty cycle drift towards PWM_MAX until it reaches 99.2% or the panel current
        	//exceeds MPPT_CURRENT_MIN, again.
        	if (pwm < PWM_MAX - 4){
				pwm_stepUp();
				//make sure that the MPPT algorithm decides towards rising PWM in the next iteration.
				MPPT_direction_up = 0xFF;
				/*
				 * setting dcdc_power_new to zero here will cause dcdc_power to become zero at the end
				 * of the function. That helps, to let the MPPT algorithm skip the "is old_power larger
				 * than new power" check and will prevent it from inverting MPPT_direction_up.
				 */
				dcdc_power_new = 0;
        	}
        }
        else {

            // start MPPT
            if (dcdc_power > dcdc_power_new) {
//                pwm_delta = -pwm_delta;
            	//toggle the direction for the next PWM change
            	MPPT_direction_up ^= 0xFF;
            }
            if (MPPT_direction_up){
            	pwm_stepUp();
            }
            else {
            	pwm_stepDown();
            }
        }
    }

    dcdc_power = dcdc_power_new;
}
