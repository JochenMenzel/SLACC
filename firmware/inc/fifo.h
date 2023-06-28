// SPDX-FileCopyrightText: 2023 2012 Frank Bättermann (frank@ich-war-hier.de)
// SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _FIFO_H__
#define _FIFO_H__

#include <util/atomic.h>

/*
Based on http://www.rn-wissen.de/index.php/FIFO_mit_avr-gcc
*/

typedef struct
{
	uint8_t size;      // buffer size
	uint8_t volatile count;     // number of characters in buffer
	uint8_t* pread;    // read-pointer
	uint8_t* pwrite;   // write-pointer
	uint8_t read2end;  // characters till read-pointer overflow
	uint8_t write2end; // characters till write-pointer overflow
} fifo_t;


void fifo_init(fifo_t*, uint8_t* buf, const uint8_t size);
uint8_t fifo_put(fifo_t* f, const uint8_t data);
void fifo_put_wait(fifo_t* f, const uint8_t data);
int16_t fifo_get(fifo_t*);
uint8_t fifo_get_wait(fifo_t*);


// This inline function doesn't check if there is sufficient space in the buffer.
// Use fifo_put or fifo_put_wait instead.
static inline void _inline_fifo_put(fifo_t* f, const uint8_t data)
{
    uint8_t* pwrite = f->pwrite;
    *(pwrite++) = data; // write data

    uint8_t write2end = f->write2end;
    if (--write2end == 0)
    {
        // wrap write pointer to begin of buffer
	    write2end = f->size;
	    pwrite -= write2end;
    }
    
    f->write2end = write2end;
    f->pwrite = pwrite;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        f->count++;
    }
}


static inline uint8_t _inline_fifo_get(fifo_t *f)
{
	uint8_t data;
    uint8_t *pread = f->pread;
    data = *(pread++);
    uint8_t read2end = f->read2end;

    if (--read2end == 0)
    {
        // wrap read pointer to end of buffer
	    read2end = f->size;
	    pread -= read2end;
    }

    f->pread = pread;
    f->read2end = read2end;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        f->count--;
    }
	return data;
}

#endif /* _FIFO_H__ */

