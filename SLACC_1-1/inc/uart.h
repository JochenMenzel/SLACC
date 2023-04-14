// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
// SPDX-FileCopyrightText: 2023 CPRGHT_BM
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _UART_H__
#define _UART_H__

#include <avr/io.h>
#include <stdint.h>

/*
Interrupt controlled interface to hardware UART0.

With help from:
- http://www.rn-wissen.de/index.php/UART_mit_avr-gcc
- http://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Der_UART
*/


// Buffer sizes
#define UART_BUFFER_RECEIVE 32
#define UART_BUFFER_SEND    64


// Baudrate register calculations
#define UART_BAUD       38400UL
#define UART_UBRR_VAL   ((F_CPU + UART_BAUD * 8) / (UART_BAUD * 16) - 1)  // clever runden
#define UART_BAUD_REAL  (F_CPU / (16 * (UART_UBRR_VAL + 1)))    // Reale Baudrate
#define UART_BAUD_ERROR ((UART_BAUD_REAL * 1000) / UART_BAUD)   // Fehler in Promille, 1000 = kein Fehler.
#if ((UART_BAUD_ERROR < 990) || (UART_BAUD_ERROR > 1010))
    #error "Baudrate error is greater than 1 percent!"
#endif 


void uart_init(void);
int16_t uart_getc(void);
uint8_t uart_getc_wait(void);
void uart_putc(const char c);
void uart_puts(char *s);
void uart_puts_P(const char *s);


// Wait until all data is sent.
static inline void uart_flush (void)
{
	while (UCSR0B & (1 << UDRIE0)) {}
}


/*
// unbuffered functions...
void uart_init(void);
void uart_putc(const char c);
void uart_puts(char *s);
void uart_puts_P(char *s);
*/

#endif

