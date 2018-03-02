#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
//#include <string.h>
//#include "xtoa.h"
#include "adc.h"
#include "load.h"
#include "led.h"
#include "main.h"
#include "uart.h"
#include "datetime.h"
#include "pwm.h"
#include "measurement.h"
#include "ST7032-master/ST7032.h"
#include "hmi.h"

/*
SLACC - Solar lead acid charge controller firmware
Frank Bättermann (frank.baettermann@ich-war-hier.de)

fixes and optimizations by Dipl.-Ing. Jochen Menzel in July 2017

TODO: On the fly frequency switching not implemented, yet!

Fuse settings:
efuse: 0xFC
hfuse: 0xDF
lfuse: 0xF7


TODO in next HW revision:
- 24V input voltage (12V regulator for mosfet-driver)
- Ability do shut down 180 deg phase for I_Charge < 1A (efficiency)
- 3.3V design with XMEGA (12 bit ADC, 10 Bit pwm, PLL?)
- Ability to cut supply for voltage/current sensing
- Filter AVCC
- Ability to cut supply for sd-card (remove separate 3.3V regulator)
- Add voltage/current lowpass with op amp
- Change charge current sensing to about 16A (shunt)
- Measure current for load-drop and drop depending on actual voltage & current
- MINI SMD VERSION? (smd mosfet, schottky diode, single phase, max 1A smd power inductor)
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
    //set our internal second-clock to zero. Thereby, we remember when we stopped charging:
    // - we want to go to sleep after 15s.
    datetime_set(0);
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



//turn on / off fan connected via LED charge port
//(meaning: we did not solder the green "charging" LED to the board, but a BSS138 n-channel mosfet
//that switches a 80mm 24V fan. 24V fans run on 12V but nicely slow and quiet.)
void fan_on(void){
    // on
    LOAD_PORT |= 1 << LOAD_BIT;

}

void fan_off(void){
    // off
    LOAD_PORT &= ~(1 << LOAD_BIT);
}

void PTC_ADCref_on(void){
	// JMe hijacked the AtMega328's pin formerly connected to the red side of the DUO LED LD1
	// set port bit to turn on power to the two PTC temperature sensors
	LED_U_RED_PORT |= 1 << LED_U_RED;
}

void PTC_ADCref_off(void){
	// JMe hijacked the AtMega328's pin formerly connected to the red side of the DUO LED LD1
	// clear port bit to turn off power to the two PTC temperature sensors
	LED_U_RED_PORT &= ~(1 << LED_U_RED);
}


/*
 * activate watchdog, shutdown all other stuff and go to sleep
 */
void goToSleep (void){
	//disable anything that uselessly burns power during sleep

	//disable the ADC
	adc_disable();
	//power down the ADC
	PRR |= (1<<PRADC);

	//GPIO
	//turn off power to the PTC temperature sensors
	PTC_ADCref_off();
	//timers?

	//disable interrupts
	cli();
	//reset watchdog-timer
	wdt_reset();
	// tell MCU that we legitimately want to change the watchdog configuration
	WDTCSR = (1<<WDCE) | (1<<WDE);
	//activate watchdog as interrupt source that triggers WDT ISR in 8 seconds.
	WDTCSR = (1<<WDIE) | (1<<WDP3) | (1<<WDP0);

	//select power down sleep mode. This sleep mode stops main clock, timers, MCU,..
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	//enable interrupts
	sei();
	//deactivate brown-out detector
	MCUCR = (1<<BODSE) | (1<<BODS);
	MCUCR = (1<<BODS);
	//go to sleep.
	sleep_mode();

	//disable the watchdog to prevent unwanted watchdog interrupts.
	wdt_disable();

	//restore power to the PTC temperature sensors and the external ADC reference voltage source
	PTC_ADCref_on();

	//wait 2ms for reference and PTC temperature sensor signals to stabilize
	_delay_us(2500);

	//power up the ADC
	PRR &= !(1<<PRADC);
	//initialize ADC
    adc_init(adc_voltageReferenceAref, adc_adjustResultRight, adc_interruptDisabled, adc_autoTriggerDisabled,\
    		 adc_autoTriggerSourceFreeRunning);
	//re-activate ADC
    adc_enable();
}

ISR(WDT_vect) {
wdt_reset(); // reset watchdog counter
//WDTCSR |= (1<<WDIE); // reenable interrupt to prevent system reset
// update second clock by 8s.
datetime_set(datetime_getS()+8);
}

void power_twi_spi_usart_disable(void){
	PRR |= (1<<PRTWI) | (1<<PRSPI) | (1<<PRUSART0);
}

int main(void)
{
	uint16_t countToDisplayUpdate = 0;

    // initialization
    led_init();
    PTC_ADCref_on();
    pwm_init();
    datetime_init();
    load_init();
    load_disconnect();
    analog_comparator_disable();
    adc_init(adc_voltageReferenceAref, adc_adjustResultRight, adc_interruptDisabled, adc_autoTriggerDisabled,\
    		 adc_autoTriggerSourceFreeRunning);
    adc_enable();
	#ifdef DEBUG_UART
    	uart_init();
	#endif
    // disable unneeded peripherals
    power_twi_spi_usart_disable();
    // initialize I2C communication and display
    ST7032init(16, 2, LCD_5x8DOTS);

    // enable interrupts
    sei();

    // say hello
//    uart_puts_P(PSTR(FIRMWARE_STRING " " FIRMWARE_VERSION_STRING "\n"));

    //wait some time to let display finish initialisation
    _delay_us(2200);

    ST7032setContrast(5);
    _delay_us(30);

    // main loop
    for (;;){
        measure();
		#ifdef USE_LOAD_SWITCH
			// connect/disconnect load
			if ((chargerStatus & chargerStatus_loadConnected) && measurements.batteryVoltage.v < BATT_U_LOAD_DROP){
				chargerStatus &= ~chargerStatus_loadConnected;
				load_disconnect();
				#ifdef DEBUG_UART
					uart_puts_P(PSTR("Status: Load disconnected (battery voltage too low)\n"));
				#endif
			}

			else if (!(chargerStatus & chargerStatus_loadConnected) && measurements.batteryVoltage.v >= BATT_U_LOAD_RECONNECT){
				chargerStatus |= chargerStatus_loadConnected;
				load_connect();
				#ifdef DEBUG_UART
					uart_puts_P(PSTR("Status: Load connected (battery voltage sufficient)\n"));
				#endif
			}
		#endif
        // Detect overtemperatures
        if (measurements.temperature1.v != UINT16_MAX && measurements.temperature1.v >= TEMP1_SHUTDOWN)
            chargerStatus |= chargerStatus_overtemperature1;
        if (measurements.temperature2.v != UINT16_MAX && measurements.temperature2.v >= TEMP2_SHUTDOWN)
            chargerStatus |= chargerStatus_overtemperature2;

    	// Clear battery full-bit (charge to U_max next time)?
    	// Independent from charging status.
    	if (measurements.batteryVoltage.v <= BATT_U_RECHARGE)
            chargerStatus &= ~chargerStatus_full;

    	// control battery charging
	    if (chargerStatus & chargerStatus_charging){
	        // currently charging...
	        if (chargerStatus & chargerStatus_overtemperature1){
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Overtemperature 1, stopped charging\n"));
				#endif
                stopCharging();
            }
            else if (chargerStatus & chargerStatus_overtemperature2){
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Overtemperature 2, stopped charging\n"));
				#endif
                stopCharging();
            }
            else if ((chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v >= BATT_U_TICKLE_TOP){
                stopCharging();
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Reached tickle charge top; stopped charging.\n"));
				#endif
            }
            else if (!(chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v >= BATT_U_MAX){
                stopCharging();
                chargerStatus |= chargerStatus_full; // remember that we reached max. voltage
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Battery full, stoppend charging.\n"));
				#endif
            }
            else if (measurements.panelCurrent.v < CHARGE_PANEL_CURRENT_MIN){
                stopCharging();
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Stopped charging (panel current too low).\n"));
				#endif
            }
            else{
                // we had no reason to stop charging: control mpp
                chargeMppt();
            }
        }
        else if (chargerStatus & chargerStatus_overtemperature1
              || chargerStatus & chargerStatus_overtemperature2){
			#ifdef DEBUG_UART
            	uart_puts_P(PSTR("Status: Overtemperature.\n"));
			#endif
            // not charging: overtemperature
            if (measurements.temperature1.v <= TEMP1_RESTART)
                chargerStatus &= ~chargerStatus_overtemperature1;
            if (measurements.temperature2.v <= TEMP2_RESTART)
                chargerStatus &= ~chargerStatus_overtemperature2;
        }
	    else if (measurements.panelVoltage.v > measurements.batteryVoltage.v){
	        // not charging, but panel voltage high enough...
 	        if ((chargerStatus & chargerStatus_full) && measurements.batteryVoltage.v <= BATT_U_TICKLE_BOTTOM){
	            startCharging();
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Started tickle charging.\n"));
				#endif
	        }
	        else if (measurements.batteryVoltage.v <= BATT_U_RECHARGE){
	            startCharging();
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Started charging.\n"));
				#endif
            }
        }

	    //check if we stopped charging for more than 15s and want to go to power-save sleep
	    if (datetime_getS() >= 15) {
	    	//check if we are still not charging, again.
	    	if (!(chargerStatus & chargerStatus_charging)){
	    		//show user that we went to sleep.
	    		showSleepMessage(measurements);
	    		//shut down any ongoing stuff and go to sleep for 8s.
	    		goToSleep();
			}
		}

	    //check if we need to turn on the cooling fan
	    if ((measurements.temperature1.v >= TEMP1_FAN_ON) || ((measurements.temperature2.v >= TEMP2_FAN_ON))){
	    	fan_on();
	    }
	    else
			//check if we can turn off the cooling fan, again
			if ((measurements.temperature1.v <= TEMP1_FAN_OFF) && ((measurements.temperature2.v <= TEMP2_FAN_OFF))){
				fan_off();
			};



        //check if 500 ms have expired since the last time we updated the process values
        if(countToDisplayUpdate == 10){
        	//show stuff on display
        	showProcessValues(measurements,chargerStatus);
        	//clear counter
        	countToDisplayUpdate = 0;
        }
        else
        	countToDisplayUpdate++;
    }
    
	return 0;
}

