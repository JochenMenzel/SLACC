// SPDX-FileCopyrightText: 2023 2019 tomozh http://ore-kb.net/archives/195
//
// SPDX-License-Identifier: MIT

/* hijacked from arduino ST7032i library */

#include <stdint.h>
#include <util/delay.h>
#include "ST7032.h"
#include "SoftI2CLib/i2csoft.h"
#include <avr/pgmspace.h>

/* global variables */
    uint8_t _displayfunction = 0x00;
    uint8_t _displaycontrol= 0x00;
    uint8_t _displaymode = 0x00;
    uint8_t _numlines;
    uint8_t _currline;

// private methods

void setDisplayControl(uint8_t setBit) {
  _displaycontrol |= setBit;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void resetDisplayControl(uint8_t resetBit) {
  _displaycontrol &= ~resetBit;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void setEntryMode(uint8_t setBit) {
  _displaymode |= setBit;
  command(LCD_ENTRYMODESET | _displaymode);
}

void resetEntryMode(uint8_t resetBit) {
  _displaymode &= ~resetBit;
  command(LCD_ENTRYMODESET | _displaymode);
}

void normalFunctionSet() {
  command(LCD_FUNCTIONSET | _displayfunction);
}

void extendFunctionSet() {
  command(LCD_FUNCTIONSET | _displayfunction | LCD_EX_INSTRUCTION);
}

// public methods

void ST7032init(uint8_t cols, uint8_t lines, uint8_t dotsize) {

	_displayfunction  = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;
	_currline = 0;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// initialize software-stack for I2C communication to display
	SoftI2CInit();
	_delay_ms(40);               // Wait time >40ms After VDD stable

	// finally, set # lines, font size, etc.
	normalFunctionSet();

	extendFunctionSet();
	command(LCD_EX_SETBIASOSC | LCD_BIAS_1_5 | LCD_OSC_183HZ);            // 1/5bias, OSC=183Hz@3.0V
	command(LCD_EX_FOLLOWERCONTROL | LCD_FOLLOWER_ON | LCD_RAB_2_00);     // internal follower circuit is turn on
	_delay_ms(200);                                       // Wait time >200ms (for power stable)
	normalFunctionSet();

	// turn the display on with no cursor or blinking default
	//  display();
	_displaycontrol   = 0x00;//LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	setDisplayControl(LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);

	// clear it off
	ST7032clear();

	// Initialize to default text direction (for roman languages)
	//  command(LCD_ENTRYMODESET | _displaymode);
	_displaymode      = 0x00;//LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	setEntryMode(LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

void ST7032setContrast(uint8_t cont)
{
	extendFunctionSet();
	command(LCD_EX_CONTRASTSETL | (cont & 0x0f));                     // Contrast set
	command(LCD_EX_POWICONCONTRASTH | LCD_ICON_ON | LCD_BOOST_ON | ((cont >> 4) & 0x03)); // Power, ICON, Contrast control
	normalFunctionSet();
}

void ST7032setIcon(uint8_t addr, uint8_t bit) {
	extendFunctionSet();
	command(LCD_EX_SETICONRAMADDR | (addr & 0x0f));                       // ICON address
	write(bit);
	normalFunctionSet();
}

/********** high level commands, for the user! */
void ST7032clear(void)
{
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	_delay_us(2000);  // this command takes a long time!
}

void ST7032home(void)
{
	command(LCD_RETURNHOME);  // set cursor position to zero
	_delay_us(2000);  // this command takes a long time!
}

void ST7032setCursor(uint8_t col, uint8_t row)
{
	const int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}

	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void ST7032noDisplay(void) {
	resetDisplayControl(LCD_DISPLAYON);
}
void ST7032display(void) {
	setDisplayControl(LCD_DISPLAYON);
}

// Turns the underline cursor on/off
void ST7032noCursor(void) {
	resetDisplayControl(LCD_CURSORON);
}
void ST7032cursor(void) {
	setDisplayControl(LCD_CURSORON);
}

// Turn on and off the blinking cursor
void ST7032noBlink(void) {
	resetDisplayControl(LCD_BLINKON);
}
void ST7032blink(void) {
	setDisplayControl(LCD_BLINKON);
}

// These commands scroll the display without changing the RAM
void ST7032scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void ST7032scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void ST7032leftToRight(void) {
	setEntryMode(LCD_ENTRYLEFT);
}

// This is for text that flows Right to Left
void ST7032rightToLeft(void) {
	resetEntryMode(LCD_ENTRYLEFT);
}

// This will 'right justify' text from the cursor
void ST7032autoscroll(void) {
	setEntryMode(LCD_ENTRYSHIFTINCREMENT);
}

// This will 'left justify' text from the cursor
void ST7032noAutoscroll(void) {
	resetEntryMode(LCD_ENTRYSHIFTINCREMENT);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void ST7032createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

/*********** mid level commands, for sending data/cmds */

void command(uint8_t value) {
	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(ST7032_I2C_DEFAULT_ADDR | I2C_RW_WRITE);
	//send control byte for command access
	SoftI2CWriteByte(0x00); //Co = 0 -> , RS = 0, R/!W = 0.
	//send command
	SoftI2CWriteByte(value);
	//send stop condition on bus
	SoftI2CStop();
}

void write(uint8_t value) {
	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(ST7032_I2C_DEFAULT_ADDR | I2C_RW_WRITE);
	//send control byte for character access
	SoftI2CWriteByte(Rs); //Co = 0 -> , RS = 1, R/!W = 0.
	//send character
	SoftI2CWriteByte(value); // strange offset on EA T123A-I2C display (ascii+0x80)
	//send stop condition on bus
	SoftI2CStop();
}

// send character
inline void ST7032writeStr(char * str) {
	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(ST7032_I2C_DEFAULT_ADDR | I2C_RW_WRITE);
	//send control byte for character access
	SoftI2CWriteByte(Rs); //Co = 0 -> , RS = 1, R/!W = 0.
	for(;;){
		//check if we already reached the end of the string.
		if (*str != '\0'){
			//send characters
			SoftI2CWriteByte(*str++);
		}
		else break;
	}
	//send stop condition on bus
	SoftI2CStop();
}
