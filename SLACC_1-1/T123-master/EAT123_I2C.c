// T123 v1.0 - Arjan D. - 15 March 2015
// LiquidCrystal_I2C V2.0 - Mario H. atmega@xs4all.nl
// Mods for Chinese I2C converter board - Murray R. Van Luyn. vanluynm@iinet.net.au

#include <stdint.h>
#include <T123-master/EAT123_I2C.h>
//#include "Wire.h"
#include "SoftI2CLib/i2csoft.h"

/* global variables for this namespace */
  uint8_t _numlines;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _Addr;
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
/* ----------------------------------- */

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data (cannot be changed over I2C)
//    M,N = 1; 4x12 display 
//    G = 1; Voltage generator Vlcd = V0 - 0.8Vdd
// 3. Display on/off control: 
//    D = 1; Display on
//    C = 1; Cursor on
//    B = 1; Blinking on
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the MCU doesn't reset the LCD, so we
// can't assume that its in that state when the code starts (and the
// T123 initialisation is called).

void T123init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
	_Addr = lcd_Addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	//_backlightval = LCD_NOBACKLIGHT;

	// initialize software-stack for I2C communication to display
	SoftI2CInit();

	_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_GENERATOR;

	if (lcd_rows == 4) {
		_displayfunction |= LCD_4LINE;
	}
	if (lcd_rows == 2) {
		_displayfunction |= LCD_2LINE;
	}

	// wait some time before configuring the display
	//JMe: make it responsibility of the application to wait until the display has finished
	// its internal reset after system power up.
//	delayMicroseconds(50000);

	// set # lcd_rows, font size, etc.
	//command(LCD_FUNCTIONSET | _displayfunction);
	
	// turn the display on with no default cursor and no default blinking
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;

	//command(LCD_DISPLAYCONTROL | _displaycontrol);
	
	// Initialize default text direction left to right (increment of cursor position), freeze display
	_displaymode = LCD_ENTRYINCREMENT | LCD_ENTRYDISPLAYFREEZE;
	
	// set the entry mode
	//command(LCD_ENTRYMODESET | _displaymode);

	//transmit initialisation data to display
	// send START condition on bus
		SoftI2CStart();

		//transmit slave address and R/!W-bit
		SoftI2CWriteByte(_Addr | I2C_RW_WRITE);
		//transmit control byte for function set: Co = 0, RS = 0, R/!W = 0.
		SoftI2CWriteByte(0x00);

		//transmit function set
		SoftI2CWriteByte(LCD_FUNCTIONSET | _displayfunction);
		// transmit display on/off control
		SoftI2CWriteByte(LCD_DISPLAYCONTROL| _displaycontrol);
		// set entry mode
		SoftI2CWriteByte(LCD_ENTRYMODESET | _displaymode);

		// clear the display
		SoftI2CWriteByte(LCD_CLEARDISPLAY);

		//send stop condition on bus
		SoftI2CStop();
}

/* provide function prototype for command transfer function used in user-available high level cmds. */
void command(uint8_t);

/********** high level commands, for the user! */

// clear() - clear display, set cursor position to zero
void T123clear(){
	command(LCD_CLEARDISPLAY);
	//JMe TODO: find some other way to wait 2ms or move responsibility for waiting to higher application.
//	delayMicroseconds(2000);   // this command takes a long time!
}

// home() - set cursor position to zero
void T123home(){
	command(LCD_RETURNHOME);
}

// setCursor(col, row) - set cursor on specified col, row
void T123setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };
	if ( row > _rows ) {
		row = _rows-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// noDisplay() - turn the display off
void T123displayOff() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// display() - turn the display on
void T123displayOn() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// noCursor() - turn the cursor off
void T123cursorOff(void) {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// cursor() - turn the cursor on
void T123cursorOn(void) {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// noBlink() - turn cursor blinking off
void T123blinkOff(void) {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// blink() - turn cursor blinking on
void T123blinkOn(void) {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// scrollDisplayLeft() - scroll display to the left without changing the RAM
void T123scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

// scrollDisplayRight() - scroll display to the left without changing the RAM
void T123scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// leftToRight() - set text flow left to right
void T123leftToRight(void) {
	_displaymode |= LCD_ENTRYINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// rightToLeft() - set text flow right to left
void T123rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// autoscroll() - autoscroll display
void T123autoscrollOn(void) {
	_displaymode |= LCD_ENTRYDISPLAYSHIFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// noAutoscroll() - freeze display
void T123autoscrollOff(void) {
	_displaymode &= ~LCD_ENTRYDISPLAYSHIFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// allows us to fill the first 8 CGRAM locations with custom characters
void T123createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7

	//set character RAM address
	command(LCD_SETCGRAMADDR | (location << 3));

	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(_Addr | I2C_RW_WRITE);
	//send control byte for character access
	SoftI2CWriteByte(Rs); //Co = 0 -> , RS = 1, R/!W = 0.
	//send character
	for (int i=0; i<8; i++) {
		SoftI2CWriteByte(charmap[i]);
	}
	//send STOP condition on bus
	SoftI2CStop();
}

/*********** mid level commands, for sending data/cmds */

// send command
inline void command(uint8_t value) {

	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(_Addr | I2C_RW_WRITE);
	//send control byte for command access
	SoftI2CWriteByte(0x00); //Co = 0 -> , RS = 0, R/!W = 0.
	//send command
	SoftI2CWriteByte(value);
	//send stop condition on bus
	SoftI2CStop();
}

// send character
inline void T123write(uint8_t value) {
	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(_Addr | I2C_RW_WRITE);
	//send control byte for character access
	SoftI2CWriteByte(Rs); //Co = 0 -> , RS = 1, R/!W = 0.
	//send character
	SoftI2CWriteByte(value); // strange offset on EA T123A-I2C display (ascii+0x80)
	//send stop condition on bus
	SoftI2CStop();
}


// send character
inline void T123writeStr(char * str) {
	//send START condition on BUS
	SoftI2CStart();
	//send slave adress for write access
	SoftI2CWriteByte(_Addr | I2C_RW_WRITE);
	//send control byte for character access
	SoftI2CWriteByte(Rs); //Co = 0 -> , RS = 1, R/!W = 0.
	for(;;){
		//check if we already reached the end of the string.
		if (*str != '\0'){
			//send characters
			SoftI2CWriteByte(*str++ + 0x80); // strange offset on EA T123A-I2C display (ascii+0x80)
		}
		else break;
	}
	//send stop condition on bus
	SoftI2CStop();
}

