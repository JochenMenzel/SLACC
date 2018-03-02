/*
 * hmi.c
 *
 *  Created on: 16.02.2018
 *      Author: dermeisterr
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "xtoa.h"
#include "measurement.h"
#include "datetime.h"
#include "main.h"
#include "ST7032-master/ST7032.h"

/*
 * this function uses 5 chars for voltage, a space and 5 chars for current, e.g.
 * "10,1V 10,5A". This uses 11 Chars of a display line.
 */
void showVoltageAndCurrent(uint16_t voltage, uint16_t current){
    char bufferValue[15];
    char buffer[15];
    char outLine[15];

    //initialize outLine as zero-terminated string.
    outLine[0] = 0;

    //check if voltage is not zero.
    if(voltage != 0){

		//convert unsigned int voltage to ascii string bufferValue, use radix 10
		utoa(voltage, bufferValue, 10);

		// pad string to given length with spaces on left side
		strpad(outLine, bufferValue , 5, ' ', 0);

		outLine[3] = outLine[2];
		outLine[2] = '.';
		outLine[4] = 'V';

		strcat(outLine," ");
    }
    else { //voltage _is_ zero.
    	strcat(outLine," 0.0V ");
    };

    //convert charge current value to string
	utoa(current, bufferValue, 10);

	// do we need to show Ampere or is mA enough because we are below 1000mA?
	if(!(current < 1000)){
		//since we cannot know the number of digits the current reading now has, we pad the string to
		//five digits by adding spaces on the left side
		strpad(buffer, bufferValue , 5, ' ', 0);
		// insert decimal point and unit
		buffer[3] = buffer[2];
		buffer[2] = '.';
		buffer[4] = 'A';
		strcat(buffer," ");
	}
	else {
		//since we cannot know the number of digits the current reading now has, we pad the string to
		//five digits by adding spaces on the left side
		strpad(buffer, bufferValue , 3, ' ', 0);

		// current value must have three or less digits. Just add the unit, "mA".
		strcat(buffer,"mA ");
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

/*
 * this function shows a temperature.
 */

void showTemperature(uint16_t temperature){
    char buffer[15];

    //returns the temperature in °C padded to 3 chars or "None" if value is invalid.
	temperatureToA(temperature, buffer);

	//check if temperature value is valid
	if(temperature != UINT16_MAX){
		//add "'C " to valid temperature value in buffer
		strcat(buffer,"\xdf""C ");
	}
	//don't add "'C" if temperature value is invalid.
	else strcat(buffer," ");

    //disable interrupts
    cli();
	//show output on display
	ST7032writeStr(buffer);
    //enable interrupts
    sei();
}

/*
 * this function shows the charger state.
 */

void showState(chargerStatus_t chargerStatus){
    char buffer[15];
//    char buffer2[15];

    //initialize buffer as zero-terminated string
    buffer[0] = 0;

    //check if power electronics heat sink is too hot
	if(chargerStatus & chargerStatus_overtemperature1){
		//yes, append state "hot"
		strcat(buffer,"hot ");
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

	//disable interrupts
    cli();
	//show output on display
	ST7032writeStr(buffer);
    //enable interrupts
    sei();
}

void showProcessValues(measurements_t measurements, chargerStatus_t chargerStatus) {
	//char buffer[15];

    //disable interrupts
    cli();
	//jump display cursor to first line first char.
	ST7032home();
    //enable interrupts
    sei();

    //display battery voltage and charge current in line 1
    showVoltageAndCurrent(measurements.batteryVoltage.v, measurements.chargeCurrent.v);

    //display the heat sink temperature
    showTemperature(measurements.temperature1.v);

    // disable interrupts
    cli();
    //set cursor to start of second line; setCursor starts counting with 0.
    ST7032setCursor(0,1);
    //enable interrupts
    sei();

    //show panel voltage and current in line 2
    showVoltageAndCurrent(measurements.panelVoltage.v, measurements.panelCurrent.v);

    //show the charger's state
    showState(chargerStatus);
}

/*
 * tell user that the SLACC went to sleep to save power while there is insufficient solar power output.
 */
void showSleepMessage(measurements_t measurements){
    char bufferValue[15];
    char buffer[15];
    char outLine[15];
    uint32_t secondsInSleep;

    //initialize some strings as zero-terminated string.
    outLine[0] = 0;
    buffer[0] = 0;

	//disable interrupts
    cli();
	//clear display
	ST7032clear();
    //enable interrupts
    sei();

    //check if voltage is not zero.
    if(measurements.batteryVoltage.v != 0){

		//convert unsigned int voltage to ascii string bufferValue, use radix 10
		utoa(measurements.batteryVoltage.v, bufferValue, 10);

		// pad string to given length with spaces on left side
		strpad(outLine, bufferValue , 5, ' ', 0);

		outLine[3] = outLine[2];
		outLine[2] = '.';
		outLine[4] = 'V';

		strcat(outLine," ");
    }
    else { //voltage _is_ zero.
    	strcat(outLine," 0.0V ");
    };

	//check if voltage is not zero.
	if(measurements.panelVoltage.v != 0){

		//convert unsigned int voltage to ascii string bufferValue, use radix 10
		utoa(measurements.panelVoltage.v, bufferValue, 10);

		// pad string to given length with spaces on left side
		strpad(buffer, bufferValue , 5, ' ', 0);

		buffer[3] = buffer[2];
		buffer[2] = '.';
		buffer[4] = 'V';
	}
	else { //voltage _is_ zero.
		strcat(buffer," 0.0V ");
	};
	// join strings that show panel voltage and battery voltage
	strcat(outLine,buffer);

    //disable interrupts
    cli();
	//show battery voltage and panel voltage, they are in outLine as strings
	ST7032writeStr(outLine);
	//enable interrupts
	sei();

	//show temperature
	showTemperature(measurements.temperature1.v);

	//disable interrupts
	cli();
    //set cursor to start of second line; setCursor starts counting with 0.
    ST7032setCursor(0,1);
    //enable interrupts
    sei();

    //now show the time we spent in sleep, so far.
    //clear outLine buffer
    outLine[0] = 0;
    //get time in sleep
    secondsInSleep = datetime_getS();

    //check if we spent more than one hour in sleep
    if(secondsInSleep > 3600) {
		//convert unsigned long second-clock to ascii string buffer, use radix 10
		utoa(secondsInSleep / 3600, buffer, 10);
		//copy number of hours to outLine buffer
		strcat(outLine,buffer);
		strcat(outLine,"h ");
		//reduce time to show by the hours
		secondsInSleep = secondsInSleep % 3600;
    };

    //now check if we spent more than one minute in sleep
    if(secondsInSleep > 60){
    	//convert unsigned long second-clock to ascii string buffer, use radix 10
		utoa(secondsInSleep / 60, buffer, 10);
		strcat(outLine,buffer);
		strcat(outLine,"' ");
		//reduce time to show by the minutes
		secondsInSleep = secondsInSleep % 60;
    };

    //show number of seconds in sleep (or the rest, after we told the user about hours and minutes)
    utoa(secondsInSleep, buffer, 10);
	strcat(outLine,buffer);
	strcat(outLine,"\" zZZ.");

	//disable interrupts
    cli();
    //show seconds since entering sleep mode
    ST7032writeStr(outLine);
    //enable interrupts
    sei();
}
