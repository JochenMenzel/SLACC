// Copyright (c) 2017 Martin Jäger (www.libre.solar)
// SPDX-FileCopyrightText: 2023 2019 Dr. Martin Jäger (https://libre.solar)
//
// SPDX-License-Identifier: Apache-2.0

// settings: battery_capacity, charge_profile
// input:  actual charge current, actual battery voltage
// output: SOC, target_voltage, target_current (=max current)

#ifndef CHARGECONTROLLER_H
#define CHARGECONTROLLER_H

// #include "mbed.h"
//#include <stdint.h>
//#include <stdlib.h>
#include "measurement.h"
#include "main.h"

// default values for 12V lead-acid battery
typedef struct {
    // State: Standby
    int time_limit_recharge; // sec
    uint16_t battery_voltage_recharge; // V
    uint16_t battery_voltage_absolute_min; // V   (under this voltage, battery is considered damaged)

    // State: CC/bulk
    uint16_t charge_current_max;  // A        PCB maximum: 20 A

    // State: CV/absorption
    uint16_t battery_voltage_max;        // max voltage per cell
    int time_limit_CV; // sec
    uint16_t current_cutoff_CV; // A

    // State: float/trickle
    uint16_t battery_voltage_trickle;    // target voltage for trickle charging of lead-acid batteries
    int time_trickle_recharge;     // sec

//    float battery_voltage_load_disconnect;
//    float battery_voltage_load_reconnect;

    // system control
    uint16_t charge_panel_current_min; // [mA] is the minimum solar panel current to continue charging.
    uint8_t restart_charging_time; // [s]	is the time between
    uint16_t mppt_panel_current_min;	// [mA] is the minimum panel current needed to do MPPT.
} ChargingProfile;

// possible charger states
enum charger_states {CHG_IDLE, CHG_CC, CHG_CV, CHG_TRICKLE};


/** Create Charge Controller object
 *
 *  @param profile ChargingProfile struct including voltage limits, etc.
 */
void charger_init(ChargingProfile *profile);

/** initialize charger profile object
 *
 * @param *profile ChargingProfile struct including voltage limits etc.
 */
void profile_init(ChargingProfile *profile);

/** encapsulate charger status
 * returns chargerStatus_charging if charging and 0 else.
 */
uint8_t isCharging(void);

/** encapsulate system status
 * returns chargerStatus_t chargerStatus.
 */
chargerStatus_t getChargerStatus(void);

/** set overtemperature1 flag in charger status
 *
 */
void setOvertemperature1(void);

/** clear overtemperature1 flag in charger status
 *
 */
void clearOvertemperature1(void);

/** set overtemperature1 flag in charger status
 *
 */
void setOvertemperature2(void);

/** clear overtemperature1 flag in charger status
 *
 */
void clearOvertemperature2(void);

uint8_t isOvertemperature1(void);

uint8_t isOvertemperature2(void);


/** guess a pwm start value from given panel- and battery voltages,
 * switch on the buck converters and
 * update charger status flag
 *
 */
void startCharging(void);

/** stop the buck converters
 *  update charger status flag and
 *  reset the datetime to 0s
 */
void stopCharging(void);

/** Get target battery current for current charger state
 *
 *  @returns
 *    Target current (A)
 */
uint16_t charger_read_target_current(void);

/** Get target battery voltage for current charger state
 *
 *  @returns
 *    Target voltage (V)
 */
uint16_t charger_read_target_voltage(void);

/** Determine if charging of the battery is allowed
 *
 *  @returns
 *    True if charging is allowed
 */
uint8_t charger_charging_enabled(void);

#ifdef USE_LOAD_SWITCH
/** Determine if discharging the battery is allowed
 *
 *  @returns
 *    True if discharging is allowed
 */
uint8_t charger_discharging_enabled(void);
#endif

/** Charger state machine update, should be called exactly once per second
 *
 *  @param battery_voltage Actual measured battery voltage (V)
 *  @param battery_current Actual measured battery current (A)
 */
void charger_update(measurements_t * measurements);

/** Get current charge controller state
 *
 *  @returns
 *    CHG_IDLE, CHG_CC, CHG_CV, CHG_TRICKLE or CHG_EQUALIZATION
 */
int charger_get_state(void);


#endif // CHARGER_H
