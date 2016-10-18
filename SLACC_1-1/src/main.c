#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "adc.h"
#include "csv.h"
#include "load.h"
#include "led.h"
#include "main.h"
#include "uart.h"
#include "datetime.h"
#include "pwm.h"
#include "fifo.h"
#include "xtoa.h"
#include "measurement.h"


/*
SLACC - Solar lead acid charge controller firmware
Frank Bättermann (frank.baettermann@ich-war-hier.de)

TODO: On the fly frequency switching not implemented, yet!

Fuse settings:
efuse: 07
hfuse: d9
lfuse: f7


TODO in next HW revision:
- 24V input voltage (12V regulator for mosfet-driver)
- Ability do shut down 180 deg phase for I_Charge < 1A (efficiency)
- 3.3V design with XMEGA (12 bit ADC, 10 Bit pwm, PLL?)
- Ability to cut supply for voltage/current sensing
- Filter AVCC
- Ability to cut supply for sd-card (remove seperate 3.3V reulator)
- Add voltage/current lowpass with op amp
- Change charge current sensing to about 16A (shunt)
- Measure current for load-drop and drop depending on actual voltage & current
- MINI SMD VERSION? (smd mosfet, shottky diode, single phase, max 1A smd power inductor)
*/


chargerStatus_t chargerStatus = chargerStatus_idle;
mppt_direction_t pwmDirection = mppt_direction_none;
uint32_t lastMppTime; // last time we adjusted the pwm
uint32_t lastPanelPower = 0;
uint32_t lastChargePower = 0;


void startCharging(void)
{
    chargerStatus |= chargerStatus_charging;
    lastPanelPower = 0;
    lastChargePower = 0;
    lastMppTime = datetime_getS();
    
    // Guess initial pwm setting: pwm = batteryVoltage * PWM_TOP / panelVoltage + offset
    uint16_t pwm = (uint16_t)(((uint32_t)measurements.batteryVoltage.v * PWM_TOP) / measurements.panelVoltage.v) + PWM_INIT_OFFSET;

    pwmDirection = mppt_direction_none;
    if (pwm > PWM_MAX)
        pwm = PWM_MAX;
    if (pwm < PWM_MIN)
        pwm = PWM_MIN;
    pwm_set(pwm);
    pwm_enable();
}


void stopCharging(void)
{
    chargerStatus &= ~chargerStatus_charging;
    lastMppTime = 0;
    pwm = 0;
    pwm_disable();
}


// Control maximum power point charging
void chargeMppt(void)
{
    uint32_t uptime = datetime_getS();
    
    // check if it's time to adjust mppt
    if (uptime < lastMppTime)
        lastMppTime += MPPT_INTERVAL; // overflow
    if (uptime - lastMppTime >= MPPT_INTERVAL)
    {
        lastMppTime = uptime;

        if (measurements.panelCurrent.v < MPPT_CURRENT_MIN)
        {
            // do not make use of MPPT because there seems to be little sun
            pwm = UINT8_MAX;
            pwmDirection = mppt_direction_pwmUp;
        }
        else
        {
            // MPPT

            switch (pwmDirection)
            {
                case mppt_direction_pwmUp:
                    if (measurements.panelPower < lastPanelPower)
                    {
                        pwmDirection = mppt_direction_pwmDown;
                        pwm_stepDown();
                    }
                    else
                        pwm_stepUp();
                    break;
            
                case mppt_direction_pwmDown:
                    if (measurements.panelPower < lastPanelPower)
                    {
                        pwmDirection = mppt_direction_pwmUp;
                        pwm_stepUp();
                    }
                    else
                        pwm_stepDown();
                    break;
            
                default: // fall through
                case mppt_direction_none:
                    // First iteration after enable or low light condition.
                    // Do nothing but changing the direction and read another
                    // pair of voltages and currents to fill lastPower-values.
                    pwmDirection = mppt_direction_pwmUp;
                    break;
            }
        }
        // remember last power
        lastPanelPower = measurements.panelPower;
        lastChargePower = measurements.chargePower;
    }
}


int main(void)
{
    // initilization
    led_init();
    pwm_init();
    datetime_init();
    load_init();
    load_disconnect();
    adc_init(adc_voltageReferenceAref, adc_adjustResultRight, adc_interruptDisabled, adc_autoTriggerDisabled, adc_autoTriggerSourceFreeRunning);  
    adc_enable();
    uart_init();

    // disable unneeded peripherals
    power_twi_disable();
 
    // enable interrupts
    sei();

    // say hello
    uart_puts_P(PSTR(FIRMWARE_STRING " " FIRMWARE_VERSION_STRING "\n"));
    
    // csv style output
    csv_init();
    csv_writeHeader();

   
    // main loop
    for (;;)
    {
        measure();
        
    	// connect/disconnet load
	    if ((chargerStatus & chargerStatus_loadConnected) && measurements.batteryVoltage.v < BATT_U_LOAD_DROP)
	    {
            chargerStatus &= ~chargerStatus_loadConnected;
            load_disconnect();
#ifdef DEBUG_UART
            uart_puts_P(PSTR("Status: Load disconnected (battery voltage too low)\n"));
#endif
	    }
	    else if (!(chargerStatus & chargerStatus_loadConnected) && measurements.batteryVoltage.v >= BATT_U_LOAD_RECONNECT)
	    {
            chargerStatus |= chargerStatus_loadConnected;
            load_connect();
#ifdef DEBUG_UART
            uart_puts_P(PSTR("Status: Load connected (battery voltage sufficient)\n"));
#endif
	    }

        // Detect overtemperatures
        if (measurements.temperature1.v != UINT16_MAX && measurements.temperature1.v >= TEMP1_SHUTDOWN)
            chargerStatus |= chargerStatus_overtemperature1;
        if (measurements.temperature2.v != UINT16_MAX && measurements.temperature2.v >= TEMP2_SHUTDOWN)
            chargerStatus |= chargerStatus_overtemperature2;
        if (measurements.temperature3.v != UINT16_MAX && measurements.temperature3.v >= TEMP3_SHUTDOWN)
            chargerStatus |= chargerStatus_overtemperature3;
    	
    	// Clear battery full-bit (charge to U_max next time)?
    	// Independent from charging status.
    	if (measurements.batteryVoltage.v <= BATT_U_RECHARGE)
            chargerStatus &= ~chargerStatus_full;

    	// control battery charging
	    if (chargerStatus & chargerStatus_charging)
	    {
	        // currently charging...
	        
            if (chargerStatus & chargerStatus_overtemperature1)
            {
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Overtemperature 1, stopped charging\n"));
#endif
                stopCharging();
            }
            else if (chargerStatus & chargerStatus_overtemperature2)
            {
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Overtemperature 2, stopped charging\n"));
#endif
                stopCharging();
            }
            else if (chargerStatus & chargerStatus_overtemperature3)
            {
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Overtemperature 3, stopped charging\n"));
#endif
                stopCharging();
            }
            else if ((chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v >= BATT_U_TICKLE_TOP)
            {
                stopCharging();
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Reached tickle charge top; stopped charging.\n"));
#endif
            }
            else if (!(chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v >= BATT_U_MAX)
            {
                stopCharging();
                chargerStatus |= chargerStatus_full; // remember that we reached max. voltage
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Battery full, stoppend charging.\n"));
#endif
            }
            else if (measurements.panelCurrent.v < CHARGE_PANEL_CURRENT_MIN)
            {
                stopCharging();
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Stopped charging (panel current too low).\n"));
#endif
            }
            else
            {
                // we had no reason to stop charging: control mpp
                chargeMppt();
            }
        }
        else if (chargerStatus & chargerStatus_overtemperature1
              || chargerStatus & chargerStatus_overtemperature2
              || chargerStatus & chargerStatus_overtemperature3)
        {
#ifdef DEBUG_UART
            uart_puts_P(PSTR("Status: Overtemperature.\n"));
#endif      
            // not charging: overtemperature
            if (measurements.temperature1.v <= TEMP1_RESTART)
                chargerStatus &= ~chargerStatus_overtemperature1;
            if (measurements.temperature2.v <= TEMP2_RESTART)
                chargerStatus &= ~chargerStatus_overtemperature2;
            if (measurements.temperature3.v <= TEMP3_RESTART)
                chargerStatus &= ~chargerStatus_overtemperature3;
        }
	    else if (measurements.panelVoltage.v > measurements.batteryVoltage.v)
	    {
	        // not charging, but panel voltage high enough...

 	        if ((chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v <= BATT_U_TICKLE_BOTTOM)
	        {
	            startCharging();
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Started tickle charging.\n"));
#endif   
	        }
	        else if (measurements.batteryVoltage.v <= BATT_U_RECHARGE)
	        {
	            startCharging();
#ifdef DEBUG_UART
                uart_puts_P(PSTR("Status: Started charging.\n"));
#endif   
            }
 	        
        }
        
        led_update();
        csv_write();
    }
    
	return 0;
}

