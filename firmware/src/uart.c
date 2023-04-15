// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
// SPDX-FileCopyrightText: 2023 CPRGHT_BM
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uart.h"
#include "main.h"
#include "fifo.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>

uint8_t bufferReceive[UART_BUFFER_RECEIVE];
uint8_t bufferSend[UART_BUFFER_SEND];

fifo_t fifoReceive;
fifo_t fifoSend;


void uart_init(void)
{
    // initialize fifo buffers    
    fifo_init(&fifoReceive, bufferReceive, UART_BUFFER_RECEIVE);
    fifo_init(&fifoSend, bufferSend, UART_BUFFER_SEND);

    // deactivate interrupts temporarily
    uint8_t sreg = SREG;
    cli();

    // UCSRnA – USART Control and Status Register n A
    // nothing to do
    
    // UCSRnB – USART Control and Status Register n B
    UCSR0B = 1 << RXCIE0 // RX Complete Interrupt Enable
           | 1 << UDRIE0 // USART Data Register Empty Interrupt Enable
           | 1 << RXEN0  // Enable Receiver
           | 1 << TXEN0; // Enable Transmitter

    // UCSRnC – USART Control and Status Register n C
    UCSR0C = 1 << UCSZ01 | 1 << UCSZ00; // asynchronous 8N1 
    
    // UBRRnL and UBRRnH – USART Baud Rate Registers
    UBRR0 = UART_UBRR_VAL;

    // flush receive-buffer
    do
    {
        UDR0; // actual value unused
    }
    while (UCSR0A & (1 << RXC0));
    
    // restore interrupt settings (enable interrupts somewhere else)
    SREG = sreg;
}


// save received byte in receive fifo
ISR(USART_RX_vect)
{
    fifo_put(&fifoReceive, UDR0);
}


// Get byte from send buffer. When byte has been sent a new interrupt will be
// triggered and deactivated if send fifo is empty. The interrupt will be
// reactivated when we write to send buffer.
ISR(USART_UDRE_vect)
{
    if (fifoSend.count)
        UDR0 = fifo_get(&fifoSend);
    else
        UCSR0B &= ~(1 << UDRIE0);
}


// Read byte from receive buffer.
// Returns -1 if buffer is empty.
int16_t uart_getc(void)
{
    return fifo_get(&fifoReceive);
}


// Wait until we receive one byte and return it.
uint8_t uart_getc_wait(void)
{
    return fifo_get_wait(&fifoReceive);
}


// Write single character to send buffer.
// Returns 0 on success, 1 on failure
void uart_putc(const char c)
{
    fifo_put_wait(&fifoSend, c);
    UCSR0B |= (1 << UDRIE0); // re-enable data register empty interrupt
}


// Write null terminated string to send buffer.
void uart_puts(char *s)
{
    for (;;)
    {
        if (!*s) // null termination
            break;
        fifo_put_wait(&fifoSend, *s++);
        UCSR0B |= (1 << UDRIE0);
    }
}


// Write null terminated string from flash to send buffer.
void uart_puts_P(const char *s)
{
    for (;;)
    {
        char c = pgm_read_byte(s++);
        if (!c)
            break;
        fifo_put_wait(&fifoSend, c);
        UCSR0B |= (1 << UDRIE0);
    }
}


/*
// unbuffered functions...

void uart_init(void)
{
    // UCSRnA – USART Control and Status Register n A
    // nothing to do
    
    // UCSRnB – USART Control and Status Register n B
    UCSR0B = 1 << RXEN0  // Enable Receiver
           | 1 << TXEN0; // Enable Transmitter

    // UCSRnC – USART Control and Status Register n C
    UCSR0C = 1 << UCSZ01 | 1 << UCSZ00; // asynchronous 8N1 
    
    // UBRRnL and UBRRnH – USART Baud Rate Registers
    UBRR0 = UART_UBRR_VAL;
}


void uart_putc(const char c)
{
    // wait until transmission possible
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = c;
}
 

void uart_puts(char *s)
{
    // send until we hit a '\0'
    while (*s)
    {
        uart_putc(*s);
        s++;
    }
}


// read string from flash send it
void uart_puts_P(char *s)
{
    for (;;)
    {
        char c = pgm_read_byte(s++);
        if (!c)
            break;
        uart_putc(c);
    }
}
*/

