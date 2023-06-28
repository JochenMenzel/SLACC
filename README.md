<!--
SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Intro

This project extends the Solar Lead Acid Charge Controller originally developed by Frank Bättermann, see [here](https://www.ich-war-hier.de/elektronik/elektronik-2012/slacc-solar-lead-acid-charge-controller/).
It uses an early version of  the charger state machine and MPPT tracker from the libre solar project, see [here](https://github.com/LibreSolar/charge-controller-firmware).

The combined key design features are:

* input voltage up to 40V
* nominal output voltage 12V
* maximum output current 10A
* maximum power point tracking of the connected PV module
* two step-down power stages operating 180° phase-shifted for less strain on filter capacitors
* over current- and over temperature protection
* display voltages, currents, temperatures, status
* nightly power-saving mode

# intended audience

This project addresses developers with a background in embedded systems and power electronics. Feedback and contributions are welcome! 

If you decide to built a solar charge controler, take a look at the libre solar project, first. The designs published there are more up-to-date than this project.

# real-life experiences

I built three SLACCs: Two are in my weekender since 2019. They work great on my two PV modules rated for 130W STC. The highest output i saw was 108W which is close to maximum due to the actual 800 W/m² of incident sunlight power in mid-Germany. The SLACCs operated under that load for approx. 1.5 hours without problems.
The third SLACC is a backup for the two in use, and may serve as test hardware if i need to debug something.
Please find images of the builds in the pictures folder.

In my camper, i did not want to use the load switch output of the SLACC because this would shut down the lighting if the batteries run low. Since i cheaply source SLA batteries from a friendly junkyard, i rather risk deep-discharge than to suddenly sit in the dark. Therefore i did not populate the load power switch and rather reused the MCU pin for fan control. Also, i did not want to log to an SD Card and therefore did not populate the associated parts.

I noticed early that the SLACCs tend to slowly drain the batteries over night. In low light conditions, e.g. fall and winter in mid-germany, this resulted in a net battery drain even though the PV panels produced power during the day. To improve this situation, i adopted libre solar's charge strategy, added MCU sleep mode and made some hardware changes to reduce idle power consumption from approximately 22mA * 12V to slightly less than 3 mA * 5V.

EMI turned out to be a problem for the I2C display: I2C is notorically prone to EMI-induced errors. I use 1.5m 4-wire PU cable to detach the displays from the SLACCs. This is enough to pick up some switching noise from the power stages. When the SLACC operates at more than approx. 7A, i sometimes see garbled display content. I took some measures to reduce EMI.

Full load tests on the bench showed that the power MosFETs run rather cool. Instead, the output capacitors turned quite hot. 

It is a good idea to do tests on the voltage and current measurement of each individual device and then correct the linearization tables in the source code: This way you eliminate slight inaccuracies introduced by the parts tolerances of the shunts, current amplifiers and associated passives.

All the parts needed to build one cost me approx. 50€; This is exclusive the fan, the housing, the emergency off switch, the Anderson PowerPole connectors, the MC4 connectors and the 4-wire M8 PU cable and M8-plug: These parts i recycled from junk. There is some cost saving potential by ordering parts for several SLACCs at once and there is also some cost saving potential in the design. For example i ordered large heatsinks for the MosFETs. These are not necessary, however; you can save 5€ per SLACC by reusing e.g. a Pentium II CPU heatsink from a scrapyard.

# hardware changes

Hare are the changes i made to the original board design SLACC 1-1 by Frank Bättermann:

* add an alphanumeric 16x2 characters I2C display 
    * I used the ST7032 COG, they are cheap, have good library support and consume litte power
    * connect SDA on PD7 and SCL on PD6 of the MCU. The remarks "yellow wire" and "green wire" in the source code refer to industrial grade four-wire PU cable with M8 connector and a M8 socket with the same wire colors. I used these for detaching the Displays from my SLACCs.

* current and temperature measurement:
    * INA168 current amplifiers
    * use 16 mOhm shunt for input current- and 6 mOhm shunt for output current measurement.
    * the supply voltage of the temperature sensors is not +5.000V but 4,89..V (what ever your stepdown voltage regulator decides to output..). Solder 10kOhm resistors to R24 and R25 to measure the sensor supply voltage via a 1:1 voltage divider. See firmware chapter for the rest of the fix.

* Power saving:
    * fake-TO220 step-down voltage regulator for the +5V supply. e.g. search ebay for "DC-DC Buck  Converter Adjustable Mini Step-down Module 1.8V 3.3V 5V 9V 12V 2A". Careful: Often, these modules have a copper bridge across the "Adjust" jumper. I did not see this, soldered the "5V" jumper and powered up the SLACC. Only after the AVR died and ceased to respond to ISP, i measured the module's output to be 9V. Better aproach: Leave the "Adjust" bridge as it is and solder the "9V" jumper. Then measure and adjust the output to 5.0V *before* you solder the module into the SLACC.
Using a step-down reulator reduced idle current consumption from 22 to 8mA, so it is a substantial improvement.
    * switchable ADC reference and temperature sensors: disconnect R23 and R19, R22 and R25 from +5V and connect to PB0, instead. This can nicely be wired via the PB0-pad of R13. Please note that i did not fully investigate yet if a switchable ADC reference and temperature sensors work as expected in all situations and if the actual power saving achieveable by this feature is relevant.
* slight improvements to efficiency:
    * switch off power stages independently for low-to-medium power single-stage operation: disconnect gate driver U$2's _SD (pin 3) from the MCU. Connect to MCU PB1, instead.
    * use Low-ESR types for all power capcitors
    * use Tantalum output capacitors

* upgrade to 40V input voltage and some EMI reduction
    * disconnect the gate drivers' power supply (pin 1 of U$1 and U$2) from Solar+ and connect to Battery+, instead
    * populate the input capacitors with 100µF 50V. Use Low-ESR!
    * populate the output capacitors C13, C14, C22, C23 with TPS7343 100/25 Tantalum 100µF 25V capacitors. Use Low-ESR!
    * add a 10µF ceramic capacitor each between GND and the connections between pins 2 of Q8/Q9 and pins 2 of Q4/Q5. Use e.g. Murata 10µF 200V. They slightly reduce input ripple and EMI because they additionally buffer the step down stages' inputs.
    * add a 1.5KE51CA TVS diode antiparallel to the solar input to block overvoltage transients
    * add a 1.5KE18CA TVS diode antiparallel to the battery output: This also prevents overvoltage damage to the SLACC if somebody disconnects the battery (or the fuse burns..) while a PV module feeds power into the SLACC. During a bench test against this scenario, i measured 13.4V and 200-300mA at the output with the TVS diode at the output. This lets the SLACC survive long enough until somebody disconnects the PV module..
    * use an aluminum housing to suppress radiated EMI
    * i built one SLACC with the 100µH 5A version of the L1 and L4 power inductors but found that inconclusive and built the next with 47µH 5A types, again. I would have liked to test 100µH 10A inductors but these are too large for the PCBs i had available at that time.

* unused Load Power Switch and SD-Card
    * do not populate IC7, IC8, the SD-Card slot, C28, C29, Q1, Q2, R3, X3 and X4.

* thermal management
    * use low-ESR capacitors for input- and output buffering
    * glue one temperature sensor to the SMD output capacitor to the top right (C22). During full load testing, on my board this one was the hottest.
    * use Q3 to drive a 24V 80 mm fan: they run nicely silent at 12V. And since we do not use the load switch stage Q1/Q2, we can use the MCU pin PD2 for the fan, instead. Direct the fan at the output capacitors.
    * the power MosFETs stay rather cool even under full load. During successive builds i used smaller heat sinks and did not see relevant heating even in the third SLACC.

* mechanics and connectivity
    * obtain a nice large red emergency off button ;) - use in my camper has also proven these switches as handy way to quickly shut down the solar system when e.g. parking under a roof.
    * use a 5A fuse for the solar input, place a solder-able car fuse holder in the SLACC's housing
    * use Anderson Powerpole 30A red and black for positive and negative lead to the battery
    * have a 10A car fuse in your camper's battery box for the SLACC (or one for each SLACC if you use several..)

# firmware reference
The firmware bases on the work of others:

* Frank Bättermann's original [source code](https://www.ich-war-hier.de/elektronik/elektronik-2012/slacc-solar-lead-acid-charge-controller/) for the SLACC
* Martin Jaeger's libre solar [firmware](https://github.com/LibreSolar/charge-controller-firmware)
* an I2C soft-stack [i2csoft](https://extremeelectronics.co.in/avr-tutorials/software-i2c-library-for-avr-mcus/)
* a display library [ST7032](http://ore-kb.net/archives/195)

# firmware changes and hints

* configuration values and thresholds are in main.h
* the original charger algorithm emphasized full-cycle use of the batteries. I.e. it let the batteries drain down to approx. 10% SOC, then recharged to 100%SOC. This is undesireable in combination with solar power: During mornings, the batteries reached full charge and the SLACC turned off. Then, during the day, the SLACC did not route solar power to water pump, refridgerator cooling fan, lighting etc. but instead let me drain the batteries. As a fix, i adopted the [charge state machine](https://libre.solar/charge-controller-firmware/src/overview/charger_concepts.html#charger-state-machine) from libre solar.
* for the display i used the [i2csoft](https://extremeelectronics.co.in/avr-tutorials/software-i2c-library-for-avr-mcus/) and [ST7032](http://ore-kb.net/archives/195) libraries with some modifications.
* the display shows current and voltage of PV module and battery, the temperature sensor 1 and the operational state ((idle, bulk, float). Nice, imho!
* the fan turns on at 60°C and off at 50°C. This simple bang-bang control proved quite effective and - in combination with the 24V fan running quiet on 12V - acceptably unobtrusive.
* measurements during bench testing showed a noteable difference between the temperature values shown and the actual temperatures i measured with a TC. This
is because the actual supply voltage of the sensors is not +5.00V but 4.89V (or whatever your +5V stepdown voltage regulator module decides to output.) To correct this, i have given up temperature sensor 3 and use ADC0 to measure the temperature sensor supply voltage via a voltage divider, instead. Then i use the actual sensor supply voltage for corrected ADC-to-temperature calculactions. This also slightly improved the accuracy of current- and voltage measurements.
* the main loop checks if the SLACC stopped charging for more than 15s. If so, the firmware enters a power down sleep mode and checks for PV output every 8s.
* i implemented some crude minimalist protection against reverse current flow in low-light conditions.
* i had to limit the PWM duty cycle to 99.2% because the high-side MosFET gate drivers use capacitive bootstrapping. This will cease to function at 100% duty cycle and the gate driver cannot keep the high-side MosFETs fully turned-on - a highly undesireable operational state for a stepdown converter.

# future work
* for some strange reason, i commented out the switch-off of the ADC reference voltage and the temperature sensors. I do not remember why - possibly the power saving effect was less than expected. I need to check this, again.
* there may be some efficiency gains in reducing the switching frequency in low and maybe high load conditions. The prep work for adaptive switching frequency already is in the code. Analyze the effiency input-to-output under various load conditions for two or three switching frequencies, in the future.

# disclaimer
The designs presented in this project are at "working prototype"-level. Please do not expect a design-set for a ready-made product; Use at your own risk.

I have taken care to check and respect the license situation of the works i reference. Any alledged or actual copyright violation is unintentional. If you feel that this project violates a copy right you believe to own or can proof ownership of, please contact me before you take any other steps. I will  then remove the disputed content asap.
