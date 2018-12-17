#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include "adc.h"
#include "pwr_management.h"
#include "datetime.h"

/*
 * pwr_management.c
 *
 *  Created on: 17.12.2018
 *      Author: Dipl.-Ing. Jochen Menzel
 */


void PTC_ADCref_init(void){
    // set pins as output
//    LED_CHARGE_DDR |= 1 << LED_CHARGE;
//    LED_U_RED_DDR |= 1 << LED_U_RED;
    PTC_ADCREF_DDR |= 1 << PTC_ADCREF;
//    LED_U_GREEN_DDR |= 1 << LED_U_GREEN;
}

void PTC_ADCref_on(void){
	// JMe hijacked the AtMega328's pin formerly connected to the red side of the DUO LED LD1
	// set port bit to turn on power to the two PTC temperature sensors
//	LED_U_RED_PORT |= 1 << LED_U_RED;
	PTC_ADCREF_PORT |= 1 << PTC_ADCREF;
}

void PTC_ADCref_off(void){
	// JMe hijacked the AtMega328's pin formerly connected to the red side of the DUO LED LD1
	// clear port bit to turn off power to the two PTC temperature sensors
//	LED_U_RED_PORT &= ~(1 << LED_U_RED);
	PTC_ADCREF_PORT &= ~(1 << PTC_ADCREF);
}

void goToSleep (void){
	//disable anything that uselessly burns power during sleep

	//disable the ADC
	adc_disable();
	//power down the ADC
	PRR |= (1<<PRADC);

	//GPIO
	//turn off power to the PTC temperature sensors
//	PTC_ADCref_off();

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
