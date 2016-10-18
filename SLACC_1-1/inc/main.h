#ifndef _MAIN_H__
#define _MAIN_H__

#define FIRMWARE_STRING "SLACC"
#define FIRMWARE_VERSION_STRING "v0.5"
#define FIRMWARE_DATE_STRING "2012-05-12"

// If debug is defined we will compile uart functions.
#define DEBUG_UART


// Define temperatures which trigger a charger shutdown/restart after shutdown
#define TEMP1_SHUTDOWN      27315UL + 80 * 100
#define TEMP1_RESTART       27315UL + 65 * 100
#define TEMP2_SHUTDOWN      27315UL + 80 * 100
#define TEMP2_RESTART       27315UL + 65 * 100
#define TEMP3_SHUTDOWN      27315UL + 80 * 100
#define TEMP3_RESTART       27315UL + 65 * 100


#define BATT_U_MAX              14400 // [mV] voltage for full charge
#define BATT_U_TICKLE_TOP       13800 // [mV] max voltage for tickle charging
#define BATT_U_TICKLE_BOTTOM    12500 // [mV] restart tickle charging here
#define BATT_U_RECHARGE         12400 // [mV] clear "full"-bit below this voltage (should be close to BATT_U_TICKLE_BOTTOM)
#define BATT_U_LOAD_RECONNECT   11500 // [mV] 
#define BATT_U_LOAD_DROP        10500 // [mV] depends heavily on load!


#define MPPT_INTERVAL                   1 // [s]
#define CHARGE_PANEL_CURRENT_MIN        20 // [mA]

// TODO: On the fly switching not implemented, yet!
#define MPPT_CURRENT_MIN                300 // TODO: Replace by switching between PANEL_CURRENT_MINIMAL_F_UP and PANEL_CURRENT_NORMAL_F_DOWN!
// Efficiency depends on available current. In minimum frequency mode we do not use MPPT
#define PANEL_CURRENT_MINIMAL_F_UP      350 // [mA] minimal -> normal (125 kHz)
#define PANEL_CURRENT_NORMAL_F_DOWN     250 // [mA] normal -> minimal (970 Hz)
#define PANEL_CURRENT_NORMAL_F_UP       5850 // [mA] normal -> high (62.5 kHz)
#define PANEL_CURRENT_HIGH_F_DOWN       5300 // [mA] high -> normal (125 kHz)


typedef enum
{
    chargerStatus_idle              = 0,
    chargerStatus_charging          = 1 << 0,
    chargerStatus_full              = 1 << 1, // use tickle charging when full
    chargerStatus_loadConnected     = 1 << 2,
    chargerStatus_overtemperature1  = 1 << 5,
    chargerStatus_overtemperature2  = 1 << 6,
    chargerStatus_overtemperature3  = 1 << 7
} chargerStatus_t;


typedef enum
{
    mppt_direction_none,
    mppt_direction_pwmUp,
    mppt_direction_pwmDown,
} mppt_direction_t;


extern chargerStatus_t chargerStatus;
extern uint8_t pwm;

#endif
