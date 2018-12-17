#ifndef _MAIN_H__
#define _MAIN_H__

#define FIRMWARE_STRING "SLACC"
#define FIRMWARE_VERSION_STRING "v0.5"
#define FIRMWARE_DATE_STRING "2012-05-12"

// If debug is defined we will compile uart functions.
// #define DEBUG_UART


// Define temperatures which trigger a charger shutdown/restart after shutdown
#define TEMP1_SHUTDOWN      27315UL + 80 * 100
#define TEMP1_RESTART       27315UL + 65 * 100
#define TEMP2_SHUTDOWN      27315UL + 80 * 100
#define TEMP2_RESTART       27315UL + 65 * 100
#define TEMP3_SHUTDOWN      27315UL + 80 * 100
#define TEMP3_RESTART       27315UL + 65 * 100

#define TEMP1_FAN_ON        27315UL + 60 * 100
#define TEMP1_FAN_OFF       27315UL + 50 * 100
#define TEMP2_FAN_ON        27315UL + 60 * 100
#define TEMP2_FAN_OFF       27315UL + 50 * 100


//#define CHARGE_PANEL_CURRENT_MIN        20 // [mA]

// TODO: On the fly switching not implemented, yet!
#define MPPT_CURRENT_MIN                200 // TODO: Replace by switching between PANEL_CURRENT_MINIMAL_F_UP and PANEL_CURRENT_NORMAL_F_DOWN!
#define PANEL_CURRENT_ACTIVATE_180DEG	525 // [mA] activate 180°-phase power stage when panel current exceeds this value
#define PANEL_CURRENT_SHTDN_180DEG		475 // [mA] shut down 180°-phase power stage when panel current is below this value
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
    chargerStatus_overtemperature2  = 1 << 6
} chargerStatus_t;


typedef enum
{
    mppt_direction_none,
    mppt_direction_pwmUp,
    mppt_direction_pwmDown,
} mppt_direction_t;


//extern chargerStatus_t chargerStatus;
extern uint8_t pwm;

#endif

