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
#include "fan.h"
#include "adc.h"
#include "main.h"
#include "charger.h"
#include "mppt.h"
#include "pwr_management.h"
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
mppt- and charger code from libre solar, adapted and included here by Dipl.-Ing. Jochen Menzel in December 2018

TODO: On the fly frequency switching not implemented, yet!

Fuse settings:
efuse: 0xFC
hfuse: 0xDF
lfuse: 0xF7


TODO in next HW revision:
- 24V input voltage (12V regulator for mosfet-driver)
- Ability to shut down 180 deg phase for I_Charge < 1A (efficiency)
- 3.3V design with XMEGA (12 bit ADC, 10 Bit pwm, PLL?)
- Ability to cut supply for voltage/current sensing
- Filter AVCC
- Ability to cut supply for sd-card (remove separate 3.3V regulator)
- Add voltage/current lowpass with op amp
- Change charge current sensing to about 16A (shunt)
- Measure current for load-drop and drop depending on actual voltage & current
- MINI SMD VERSION? (smd mosfet, schottky diode, single phase, max 1A smd power inductor)
*/

#define DEBUG_UART

#ifdef DEBUG_UART
	#define DEBUG_static(z) uart_puts_P(PSTR(z));
	#define DEBUG(z) uart_puts(z);
#endif

ChargingProfile profile;

int main(void)
{
	uint16_t countToDisplayUpdate = 0;
    uint16_t last_second = 0;

    // initialization
	PTC_ADCref_init();
    PTC_ADCref_on();
    fan_init();
    fan_off();
    pwm_init();
    datetime_init();
    analog_comparator_disable();
    adc_init(adc_voltageReferenceAref, adc_adjustResultRight, adc_interruptDisabled, adc_autoTriggerDisabled,\
    		 adc_autoTriggerSourceFreeRunning);
    adc_enable();
	#ifdef DEBUG_UART
    	uart_init();
	#endif
    // disable unneeded peripherals
    power_twi_spi_usart_disable();

    /* initialize the charger profile */
    profile_init(&profile);

    /* initialize the charger state machine */
    charger_init(&profile);

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

        // Detect overtemperatures
        if (measurements.temperature1.v != UINT16_MAX && measurements.temperature1.v >= TEMP1_SHUTDOWN)
        	setOvertemperature1();
        if (measurements.temperature2.v != UINT16_MAX && measurements.temperature2.v >= TEMP2_SHUTDOWN)
            setOvertemperature2();

      	// update MPPT every 1000 ms
        if(datetime_getS() - last_second >= 1){
        	last_second = datetime_getS();

        	/* update the charger state machine */
        	charger_update(&measurements);

        	update_mppt( &measurements, &profile);
        }

	    //check if we stopped charging for more than 15s and want to go to power-save sleep
	    if (datetime_getS() >= 15) {
	    	//check if we are still not charging, again.
	    	if (!isCharging()){
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
        	showProcessValues(measurements);
        	//clear counter
        	countToDisplayUpdate = 0;
        }
        else
        	countToDisplayUpdate++;
    }
    
	return 0;
}

