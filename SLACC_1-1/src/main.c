#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "xtoa.h"
#include "adc.h"
#include "load.h"
#include "led.h"
#include "main.h"
#include "uart.h"
#include "datetime.h"
#include "pwm.h"
#include "measurement.h"
#include "ST7032-master/ST7032.h"

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

void showVoltageAndCurrent(uint16_t voltage, uint16_t current){
    char bufferValue[15];
    char buffer[15];
    char outLine[15];

	utoa(voltage, bufferValue, 10);

	// pad string to given length with spaces on left side
	strpad(outLine, bufferValue , 5, ' ', 0);

	outLine[3] = outLine[2];
	outLine[2] = '.';
	outLine[4] = 'V';

	strcat(outLine," ");

	//convert charge current value to string
	utoa(current, bufferValue, 10);

	if(!(current < 1000)){
		//since we cannot know the number of digits the current reading now has, we pad the string to
		//five digits by adding spaces on the left side
		strpad(buffer, bufferValue , 5, ' ', 0);
		// insert decimal point and unit
		buffer[3] = buffer[2];
		buffer[2] = '.';
		buffer[4] = 'A';
	}
	else {
		//since we cannot know the number of digits the current reading now has, we pad the string to
		//five digits by adding spaces on the left side
		strpad(buffer, bufferValue , 3, ' ', 0);

		// current value must have three or less digits. Just add the unit, "mA".
		strcat(buffer,"mA");
	}

	//merge value into output line
	strcat(outLine,buffer);

    //disable interrupts
    cli();
	//show output metric data
	ST7032writeStr(outLine);
    //enable interrupts
    sei();
}

void showTemperatureAndState(void){
    char buffer[15];
    char buffer2[15];

	temperatureToA(measurements.temperature1.v, buffer);

	//check if temperature value is valid
	if(measurements.temperature1.v != UINT16_MAX){
		//add "'C " to valid temperature value in buffer
		strcat(buffer,"\x27""C ");
	}
	//don't add "'C" if temperature value is invalid.
	else strcat(buffer," ");

	//check if power electronics heat sink is too hot
	if(chargerStatus & chargerStatus_overtemperature1){
		//yes, append state "hot"
		strcat(buffer,"hot");
	}
	else {
		//show charger state
		switch (chargerStatus & 0x03){
		case chargerStatus_idle:
			//tell user that charger is idle
			strcat(buffer,"idle");
			break;

		case chargerStatus_charging:
			//tell user that we are bulk charging
			strcat(buffer,"bulk");
			break;

		case chargerStatus_full:
			strcat(buffer,"float");
			break;

		default:
			break;
		}
	}
	utoa(chargerStatus,buffer2,16);
	strcat(buffer,buffer2);
    //disable interrupts
    cli();
	//show output on display
	ST7032writeStr(buffer);
    //enable interrupts
    sei();
}

/*
 * 123456789012
 * 12.2V 10.1A
 * 12.2V 999mA
 * 34.1V  3.4A
 * 60°C charge
 */

void showProcessValues(void) {
//	char buffer[15];

    //disable interrupts
    cli();
	//jump display cursor to first line first char.
	ST7032home();
    //enable interrupts
    sei();

    //display battery voltage and charge current in line 1
    showVoltageAndCurrent(measurements.batteryVoltage.v, measurements.chargeCurrent.v);

    // disable interrupts
    cli();
    //set cursor to start of second line; setCursor starts counting with 0.
    ST7032setCursor(0,1);
    //enable interrupts
    sei();

    //show panel voltage and current in line 2
    showVoltageAndCurrent(measurements.panelVoltage.v, measurements.panelCurrent.v);
/*
    // disable interrupts
    cli();
    //set cursor to start of third line; setCursor starts counting with 0.
    ST7032setCursor(0,2);
    //enable interrupts
    sei();

    // show power electronics heat sink temperature and operating state of charger
    showTemperatureAndState();
    */
/*
    utoa(measurements.panelCurrent.adc,buffer,16);
    // disable interrupts
    cli();
    //set cursor to start of third line; setCursor starts counting with 0.
    ST7032writeStr(buffer);
    //enable interrupts
    sei();
*/
}

//turn on / off fan connected via LED charge port
//(meaning: we did not solder the green "charging" LED to the board, but a BSS138 n-channel mosfet
//that switches a 80mm 24V fan. 24V fans run on 12V but nicely slow and quiet.)
void fan_on(void){
    // on
    LED_CHARGE_PORT |= 1 << LED_CHARGE;
}

void fan_off(void){
    // off
    LED_CHARGE_PORT &= ~(1 << LED_CHARGE);
}


int main(void)
{
	uint16_t countToDisplayUpdate = 0;

    // initialization
    led_init();
    pwm_init();
    datetime_init();
    load_init();
    load_disconnect();
    adc_init(adc_voltageReferenceAref, adc_adjustResultRight, adc_interruptDisabled, adc_autoTriggerDisabled,\
    		 adc_autoTriggerSourceFreeRunning);
    adc_enable();
    uart_init();

    // disable unneeded peripherals
    power_twi_disable();

    // initialize I2C communication and display
    ST7032init(16, 2, LCD_5x8DOTS);

    // enable interrupts
    sei();

    // say hello
//    uart_puts_P(PSTR(FIRMWARE_STRING " " FIRMWARE_VERSION_STRING "\n"));

    //wait some time to let display finish initialisation
    _delay_us(2200);

    ST7032setContrast(5);
    _delay_us(2200);

    // main loop
    for (;;){
        measure();
        
    	// connect/disconnet load
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
            else if (chargerStatus & chargerStatus_overtemperature3){
				#ifdef DEBUG_UART
                	uart_puts_P(PSTR("Status: Overtemperature 3, stopped charging\n"));
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
              || chargerStatus & chargerStatus_overtemperature2
              || chargerStatus & chargerStatus_overtemperature3){
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
        
        //led_update();
        // csv_write();

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
        if(countToDisplayUpdate == 50){
        	//show stuff on display
        	showProcessValues();
        	//clear counter
        	countToDisplayUpdate = 0;
        }
        else
        	countToDisplayUpdate++;
    }
    
	return 0;
}

