// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
// SPDX-FileCopyrightText: 2023 CPRGHT_BM
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CSV_H__
#define _CSV_H__

#include <stdint.h>
#include <avr/io.h>

#define CSV_UART_INTERVAL       1 // [s]

#define CSV_FILE_ENABLED        1 // 0: disabled, 1: enabled
#define CSV_FILE_INTERVAL       5 // [s]
#define CSV_FILE_FLUSHINTERVAL  120 // [s]
#define CSV_FILE_MAXSIZE        1024UL * 1024 * 1024 // 1 GB
#define CSV_FILE_MINSIZE        32UL * 1024 // 32 KB


uint8_t csv_init(void);
void csv_write(void);
void csv_writeHeader(void);

// private
void _csv_uartWrite(void);
void _csv_uartWriteHeader(void);
#if (CSV_FILE_ENABLED == 1)
void _csv_fileWrite(void);
void _csv_fileWriteHeader(void);
uint8_t _csv_findFileName(char* filename);
void _csv_fileNew(void);
#endif // CSV_FILE_ENABLED

#endif

