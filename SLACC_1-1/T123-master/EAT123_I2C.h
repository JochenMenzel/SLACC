// T123 v1.0 - Arjan D. - 15 March 2015
// Original code: LiquidCrystal_I2C V2.0 - Mario H. atmega@xs4all.nl
// Mods for Chinese I2C converter board - Murray R. Van Luyn. vanluynm@iinet.net.au

#ifndef T123_h
#define T123_h

#include <inttypes.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYINCREMENT 0x02
#define LCD_ENTRYDECREMENT 0x00
#define LCD_ENTRYDISPLAYSHIFT 0x01
#define LCD_ENTRYDISPLAYFREEZE 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_4LINE 0x0C			// N=1, M=1
#define LCD_2LINE 0x08			// N=1, M=0
#define LCD_1LINE 0x00			// N=0, M=0
//#define LCD_5x10DOTS 0x04
//#define LCD_5x8DOTS 0x00
#define LCD_GENERATOR 0x02

// flags for backlight control
#define LCD_BACKLIGHT   B00001000
#define LCD_NOBACKLIGHT B00000000

//PCF2116 / EAT123-I2C bus address
#define LCD_I2CADDRESS 0x74


#define Rw 0b00100000  // Read/Write bit (1 read, 0 write)
#define Rs 0b01000000  // Register select bit
#define Co 0b10000000  // Command bit, set to one if last control byte in transmission and only data bytes follow.

  void T123init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);
  void T123clear(void);
  void T123home(void);
  void T123displayOff(void);
  void T123displayOn(void);
  void T123blinkOff(void);
  void T123blinkOn(void);
  void T123cursorOff(void);
  void T123cursorOn(void);
  void T123scrollDisplayLeft(void);
  void T123scrollDisplayRight(void);
  void T123printLeft(void);
  void T123printRight(void);
  void T123leftToRight(void);
  void T123rightToLeft(void);
  void T123autoscrollOn(void);
  void T123autoscrollOff(void);
  void T123createChar(uint8_t, uint8_t[]);
  void T123setCursor(uint8_t, uint8_t);
  void T123write(uint8_t);
  void T123writeStr(char* str);
#endif
