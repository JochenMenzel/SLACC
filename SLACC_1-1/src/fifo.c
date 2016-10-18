#include <stdint.h>
#include "fifo.h"


void fifo_init(fifo_t* f, uint8_t *buffer, const uint8_t size)
{
	f->count = 0;
	f->pread = f->pwrite = buffer;
	f->read2end = f->write2end = f->size = size;
}


// Insert data into fifo.
// Returns -1 if buffer is full and data could not be added.
uint8_t fifo_put(fifo_t *f, const uint8_t data)
{
    if (f->count >= f->size)
        return 1; // no space left
	_inline_fifo_put(f, data);
	return 0; // data sucessfully added
}


// Wait until there is space, than insert data into fifo.
void fifo_put_wait(fifo_t *f, const uint8_t data)
{
    while (f->count >= f->size) {}
	_inline_fifo_put(f, data);
}


int16_t fifo_get(fifo_t *f)
{
	if (!f->count)
	    return -1;
	return (int16_t)_inline_fifo_get(f);	
}


uint8_t fifo_get_wait(fifo_t *f)
{
	while (!f->count) {}
	return _inline_fifo_get(f);	
}


