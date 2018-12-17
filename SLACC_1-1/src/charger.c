/* mbed library for a battery charge controller
 * Copyright (c) 2017 Martin Jäger (www.libre.solar)
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

#include "charger.h"
#include "datetime.h"
#include "measurement.h"
#include "main.h"
#include "pwm.h"

static ChargingProfile *_profile;          // all charging profile variables
static int _state;                         // valid states: enum charger_states
static int _time_state_changed;            // timestamp of last state change
static uint16_t _target_voltage;              // target voltage for current state
static uint16_t _target_current;              // target current for current state
static int _time_voltage_limit_reached;    // last time the CV limit was reached
//static bool _charging_enabled;
#ifdef USE_LOAD_SWITCH
static bool _discharging_enabled;
#endif

chargerStatus_t chargerStatus = chargerStatus_idle;

// private function

/** Enter a different charger state
 *
 *  @param next_state Next state (e.g. CHG_CC)
 */
void charger_enter_state(int next_state);


void charger_init(ChargingProfile *profile)
{
    _profile = profile;
    //_charging_enabled = false;
    _state = CHG_IDLE;
    _time_state_changed = -profile->time_limit_recharge;     // start immediately
    _target_current = profile -> charge_current_max;
    _target_voltage = profile-> battery_voltage_max;
}

void profile_init(ChargingProfile *profile){

    // State: Standby
    profile-> time_limit_recharge = 60;				// sec
    profile-> battery_voltage_recharge = 13500;		// mV
    profile-> battery_voltage_absolute_min = 10000;	// mV   (under this voltage, battery is considered damaged)

    // State: CC/bulk
    profile-> charge_current_max = 10000;			// [mA}        PCB maximum: 10 A

    // State: CV/absorption
    profile-> battery_voltage_max = 14200;			// [mV] max voltage for a 6-cell lead acid battery
    profile-> time_limit_CV = 120*60; 				// [sec}
    profile-> current_cutoff_CV = 2000; 			// [mA]

    // State: profile->/trickle
    profile-> battery_voltage_trickle = 13800;  	// [mV] target voltage for trickle charging of lead-acid batteries
    profile-> time_trickle_recharge = 60*60;    	// sec

    // system control
    profile -> charge_panel_current_min = 20; 		// [mA]
    profile -> restart_charging_time = 5; 			// [s]
    profile -> mppt_panel_current_min = 100;		// [mA] is the minimum panel current needed to do MPPT.
}

//extern chargerStatus_t chargerStatus = chargerStatus_idle;


void startCharging(void)
{
    chargerStatus |= chargerStatus_charging;

    // Guess initial pwm setting: pwm = batteryVoltage * PWM_TOP / panelVoltage + offset
    uint16_t pwm = (uint16_t)(((uint32_t)measurements.batteryVoltage.v * PWM_TOP) / measurements.panelVoltage.v) + PWM_INIT_OFFSET;

    if (pwm > PWM_MAX)
        pwm = PWM_MAX;
    if (pwm < PWM_MIN)
        pwm = PWM_MIN;
    pwm_set(pwm);
    pwm_0deg_enable(pwm_frequencyHigh);
    pwm_180deg_enable(pwm_frequencyHigh);
}

void stopCharging(void)
{
    chargerStatus &= ~chargerStatus_charging;
    pwm = 0;
    pwm_0deg_disable();
    pwm_180deg_disable();
    //set our internal second-clock to zero. Thereby, we remember when we stopped charging:
    // - we want to go to sleep after 15s.
    datetime_set(0);
}

inline uint8_t isCharging(void){
	return(chargerStatus & chargerStatus_charging);
}

inline chargerStatus_t getChargerStatus(void){
	return(chargerStatus);
}

inline void setOvertemperature1(void){
	chargerStatus |= chargerStatus_overtemperature1;
}

inline void clearOvertemperature1(void){
	chargerStatus &= !chargerStatus_overtemperature1;
}

inline void setOvertemperature2(void){
	chargerStatus |= chargerStatus_overtemperature2;
}

inline void clearOvertemperature2(void){
	chargerStatus &= !chargerStatus_overtemperature2;
}

inline uint8_t isOvertemperature1(void){
	return(chargerStatus & chargerStatus_overtemperature1);
}

inline uint8_t isOvertemperature2(void){
	return(chargerStatus & chargerStatus_overtemperature2);
}

/*****************************************************************************
 *  Charger state machine

 ## Idle
 Initial state of the charge controller. If the solar voltage is high enough
 and the battery is not full, charging in CC mode is started.

 ### CC / bulk charging
 The battery is charged with maximum possible current (MPPT algorithm is
 active) until the CV voltage limit is reached.

 ### CV / absorption charging
 Lead-acid batteries are charged for some time using a slightly higher charge
 voltage. After a current cutoff limit or a time limit is reached, the charger
 goes into trickle or equalization mode for lead-acid batteries or back into
 Standby for Li-ion batteries.

 ### Trickle charging
 This mode is kept forever for a lead-acid battery and keeps the battery at
 full state of charge. If too much power is drawn from the battery, the
 charger switches back into CC / bulk charging mode.

 */

void charger_update(measurements_t * measurements) {

    //printf("_time_state_change = %d, time = %d, v_bat = %f, i_bat = %f\n", _time_state_changed, datetime_getS(), battery_voltage, battery_current);
#ifdef USE_LOAD_SWITCH
    // Load management
    if (battery_voltage < _profile->battery_voltage_load_disconnect ) {
        _discharging_enabled = false;
    }
    if (battery_voltage >= _profile->battery_voltage_load_reconnect ) {
        _discharging_enabled = true;
    }
#endif
    // state machine
    switch (_state) {

        case CHG_IDLE: {
            if  (measurements->batteryVoltage.v < _profile->battery_voltage_recharge
                 && (datetime_getS() - _time_state_changed) > _profile->time_limit_recharge)
            {
                _target_current = _profile->charge_current_max;
                _target_voltage = _profile->battery_voltage_max;

                startCharging();
                charger_enter_state(CHG_CC);
            }
            break;
        }

        case CHG_CC: {
            if (measurements->batteryVoltage.v > _target_voltage) {
                _target_voltage = _profile->battery_voltage_max;
                charger_enter_state(CHG_CV);
            }
            break;
        }

        case CHG_CV: {
            if (measurements->batteryVoltage.v >= _target_voltage) {
                _time_voltage_limit_reached = datetime_getS();
            }

            // cut-off limit reached because battery full (i.e. CV mode still
            // reached by available solar power within last 2s) or CV period long enough?
            if ((measurements->chargeCurrent.v < _profile->current_cutoff_CV && \
            		(datetime_getS() - _time_voltage_limit_reached) < 2)
               || (datetime_getS() - _time_state_changed) > _profile->time_limit_CV)
            {
				_target_voltage = _profile->battery_voltage_trickle;
				charger_enter_state(CHG_TRICKLE);
            }
            break;
        }

        case CHG_TRICKLE: {
            if (measurements->batteryVoltage.v >= _target_voltage) {
                _time_voltage_limit_reached = datetime_getS();
            }

            if (datetime_getS() - _time_voltage_limit_reached > _profile->time_trickle_recharge)
            {
                _target_current = _profile->charge_current_max;
                _target_voltage = _profile->battery_voltage_max;
                charger_enter_state(CHG_CC);
            }
            // assumtion: trickle does not harm the battery --> never go back to idle
            // (for Li-ion battery: disable trickle!)
            break;
        }
    }
}

void charger_enter_state(int next_state)
{
 //   printf("Enter State: %d\n", next_state);
    _time_state_changed = datetime_getS();
    _state = next_state;
}

#ifdef USE_LOAD_SWITCH
bool charger_discharging_enabled()
{
    return _discharging_enabled;
}
#endif

/*
bool charger_charging_enabled()
{
    return _charging_enabled;
}
*/
int charger_get_state()
{
    return _state;
}

uint16_t charger_read_target_current()
{
    return _target_current;
}

uint16_t charger_read_target_voltage()
{
    return _target_voltage;
}
