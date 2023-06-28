// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "csv.h"
#include "datetime.h"
#include "uart.h"
#include "main.h"
#include "measurement.h"
#include "pwm.h"
#include "xtoa.h"

#if (CSV_FILE_ENABLED == 1)
// AVR FAT32
// Source: http://www.mikrocontroller.net/articles/AVR_FAT32
#include "avrfat32/mmc_config.h"
#include "avrfat32/file.h"
#include "avrfat32/fat.h"
#include "avrfat32/mmc.h"
// Defines like "configure_pin_protected()" in mmc.h are fixed by now (we do not have any hardware support)
#endif // CSV_FILE_ENABLED

const char csvHeader[] PROGMEM =
    "Time[s];StatusCharge;StatusFull;StatusLoadConnected;StatusOvertemp1;StatusOvertemp1;StatusOvertemp1;PWM" 
    ";Temp1Adc;Temp1 [degK*100];Temp2Adc;Temp2 [degK*100];Temp3Adc;Temp3 [degK*100]"
    ";UBattAdc;UBatt [mV];IChargeAdc;ICharge [mA];PCharge [W*100]"
    ";UPanelAdc;UPanel [mV];IPanelAdc;IPanel [mA];PPanel [W*100]"
    ";Efficiency [%]"
    "\n";

uint32_t csvUart_lastTime; // last time we wrote csv data to the uart (whole seconds)
#if (CSV_FILE_ENABLED == 1)
uint32_t csvFile_lastTime; // last time we wrote csv data to file (whole seconds)
uint32_t csvFile_lastTimeFlushed; // last time we flushed the buffer
uint32_t csvFile_growUntil;
uint8_t csvFile_initialized = 0;
#endif // CSV_FILE_ENABLED

uint8_t csv_init(void)
{
    uint8_t ret = 0;
    uint32_t time = datetime_getS();
    
    csvUart_lastTime = time;
    
#if (CSV_FILE_ENABLED == 1)
    csvFile_lastTime = csvFile_lastTimeFlushed = time;
    
    uart_puts_P(PSTR("Sd-card: Initializing... "));
	if (mmc_init() == FALSE) // sd/mmc config
        uart_puts_P(PSTR("failed\n"));
    else
    {
        uart_puts_P(PSTR("Ok\nSd-card: Loading FAT... "));
        if (fat_loadFatData() == FALSE) // fat config
            uart_puts_P(PSTR("failed\n"));
        else
        {
            uart_puts_P(PSTR("Ok\n"));
            csvFile_initialized = 1;
        }
    
    }  

    _csv_fileNew();
#endif // CSV_FILE_ENABLED

    return ret;
}


void csv_write(void)
{
    // create a copy of the system uptime
    uint32_t uptime = datetime_getS();
      
    if (uptime < csvUart_lastTime)
    {
        // there was an overflow of uptime in between --> also overflow lastTime
        csvUart_lastTime += CSV_UART_INTERVAL;
    }
    if (uptime - csvUart_lastTime >= CSV_UART_INTERVAL)
    {
        csvUart_lastTime = uptime;
        _csv_uartWrite();
    }
    
#if (CSV_FILE_ENABLED == 1)
    if (csvFile_initialized)
    {
        if (uptime < csvFile_lastTime)
        {
            // there was an overflow of uptime in between --> also overflow lastTime
            csvFile_lastTime += CSV_FILE_INTERVAL;
        }
        if (uptime - csvFile_lastTime >= CSV_FILE_INTERVAL)
        {
            csvFile_lastTime = uptime;
            _csv_fileWrite();
        }
        
        // flush file after a given duration
        if (uptime < csvFile_lastTimeFlushed)
        {
            // there was an overflow of uptime in between --> also overflow lastTime
            csvFile_lastTimeFlushed += CSV_FILE_FLUSHINTERVAL;
        }
        if (uptime - csvFile_lastTimeFlushed >= CSV_FILE_FLUSHINTERVAL)
        {
            csvFile_lastTimeFlushed = uptime;
            fflushFileData();
        }
        else if (file.length >= csvFile_growUntil)
        {
            // switch to next file if max. size is reached
            ffclose();
            uart_puts_P(PSTR("Sd-card: Switching to next csv file\n"));
            _csv_fileNew();
            _csv_fileWriteHeader();
        }
    }
#endif // CSV_FILE_ENABLED
}


void csv_writeHeader(void)
{
    _csv_uartWriteHeader();
#if (CSV_FILE_ENABLED == 1)
    if (csvFile_initialized)
        _csv_fileWriteHeader();
#endif // CSV_FILE_ENABLED
}


// private

void _csv_uartWriteHeader(void)
{
    uart_puts_P(PSTR("\n\n"));
    uart_puts_P(csvHeader);
}


void _csv_uartWrite(void)
{
    char buffer[15];

    // time
    uart_puts(datetime_nowToS(buffer));
    // status
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_charging ? '1' : '0');
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_full ? '1' : '0');
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_loadConnected ? '1' : '0');
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_overtemperature1 ? '1' : '0');
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_overtemperature2 ? '1' : '0');
    uart_putc(';'); uart_putc(chargerStatus & chargerStatus_overtemperature3 ? '1' : '0');
    // pwm setting
    uart_putc(';'); uart_puts(utoa(pwm_get(), buffer, 10));
    // temperatures
    uart_putc(';'); uart_puts(utoa(measurements.temperature1.adc, buffer, 10));
    uart_putc(';'); uart_puts(temperatureToA(measurements.temperature1.v, buffer));
    uart_putc(';'); uart_puts(utoa(measurements.temperature2.adc, buffer, 10));
    uart_putc(';'); uart_puts(temperatureToA(measurements.temperature2.v, buffer));
    uart_putc(';'); uart_puts(utoa(measurements.temperature3.adc, buffer, 10));
    uart_putc(';'); uart_puts(temperatureToA(measurements.temperature3.v, buffer));
    // battery
    uart_putc(';'); uart_puts(utoa(measurements.batteryVoltage.adc, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.batteryVoltage.v, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.chargeCurrent.adc, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.chargeCurrent.v, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.chargePower, buffer, 10));
    // panel
    uart_putc(';'); uart_puts(utoa(measurements.panelVoltage.adc, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.panelVoltage.v, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.panelCurrent.adc, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.panelCurrent.v, buffer, 10));
    uart_putc(';'); uart_puts(utoa(measurements.panelPower, buffer, 10));
    // efficiency
    uart_putc(';'); uart_puts(efficiencyToA(measurements.efficiency, buffer));
    
    uart_puts("\n");
}

#if (CSV_FILE_ENABLED == 1)
// this is a mirror of _csv_uartWrite - it just writes to a file instead of the uart
void _csv_fileWrite(void)
{
    char buffer[15];

    // time
    ffwrites(datetime_nowToS(buffer));
    // status
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_charging ? '1' : '0');
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_full ? '1' : '0');
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_loadConnected ? '1' : '0');
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_overtemperature1 ? '1' : '0');
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_overtemperature2 ? '1' : '0');
    ffwrite(';'); ffwrite(chargerStatus & chargerStatus_overtemperature3 ? '1' : '0');
    // pwm setting
    ffwrite(';'); ffwrites(utoa(pwm_get(), buffer, 10));
    // temperatures
    ffwrite(';'); ffwrites(utoa(measurements.temperature1.adc, buffer, 10));
    ffwrite(';'); ffwrites(temperatureToA(measurements.temperature1.v, buffer));
    ffwrite(';'); ffwrites(utoa(measurements.temperature2.adc, buffer, 10));
    ffwrite(';'); ffwrites(temperatureToA(measurements.temperature2.v, buffer));
    ffwrite(';'); ffwrites(utoa(measurements.temperature3.adc, buffer, 10));
    ffwrite(';'); ffwrites(temperatureToA(measurements.temperature3.v, buffer));
    // battery
    ffwrite(';'); ffwrites(utoa(measurements.batteryVoltage.adc, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.batteryVoltage.v, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.chargeCurrent.adc, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.chargeCurrent.v, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.chargePower, buffer, 10));
    // panel
    ffwrite(';'); ffwrites(utoa(measurements.panelVoltage.adc, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.panelVoltage.v, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.panelCurrent.adc, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.panelCurrent.v, buffer, 10));
    ffwrite(';'); ffwrites(utoa(measurements.panelPower, buffer, 10));
    // efficiency
    ffwrite(';'); ffwrites(efficiencyToA(measurements.efficiency, buffer));
    
    ffwrites("\n");
}


void _csv_fileWriteHeader(void)
{
    ffwrites_P(csvHeader);
}


void _csv_fileNew(void)
{
    
    // look for sufficient space
    if (csvFile_initialized)
    {
        uint64_t freeBytes;
        char buffer[11];

        uart_puts_P(PSTR("Sd-card: Free space: "));
        freeBytes = fat_getFreeBytes();
        uart_puts(ultoa((uint32_t)freeBytes >> 20, buffer, 10)); // divide by 2^20 --> Bytes to MB
        uart_puts_P(PSTR(" MB\n"));

        if (freeBytes < CSV_FILE_MINSIZE)
        {
            uart_puts_P(PSTR("Sd-card: Insufficient space for csv file"));
            csvFile_initialized = 0;
        }
        else
        {
            
            if (freeBytes < CSV_FILE_MAXSIZE)
                csvFile_growUntil = (uint32_t)freeBytes;
            else
                csvFile_growUntil = CSV_FILE_MAXSIZE;
                
            uart_puts_P(PSTR("Sd-card: Maximum file size: "));
            uart_puts(ultoa(csvFile_growUntil >> 20, buffer, 10));
            uart_puts_P(PSTR(" MB\n"));
                
            // We cannot write to the very last byte of the card, because we
            // check the file size _after_ writing a sample.
            csvFile_growUntil -= 1024;
        }
    }

    // look for unused filename to write to
    if (csvFile_initialized)
    {
        char filename[13];

        if (_csv_findFileName(filename))
        {
            uart_puts_P(PSTR("Sd-card: Could not find unused output filename for csv-data.\n"));
            csvFile_initialized = 0;
        }
        else
        {
            uart_puts_P(PSTR("Sd-card: Csv data will be written to "));
	        uart_puts(filename);
	        uart_puts_P(PSTR("\n"));
	    }
    }
}


// Find unused filename in the form of "########.csv"; returns 0 if successful.
// File remains opened when function is left.
// TODO: speed this up by multiplying the number... (use file-exists instead of create then)
uint8_t _csv_findFileName(char* filename)
{
    char buffer_num[9];
    uint32_t filenumber = 0;
    uint8_t ret = 1; // default is no filename found
    do
    {
        ultoa(filenumber, buffer_num, 10);
        strpad(filename, buffer_num, 8, '0', 0);
        strcat_P(filename, PSTR(".csv"));
        
	    if (ffopen((uint8_t*)filename,'c') == MMC_FILE_CREATED)
	    {
	        ret = 0;
	        break;
	    }

    } while (++filenumber < 100000000);
    
    return ret;
}

#endif // CSV_FILE_ENABLED

