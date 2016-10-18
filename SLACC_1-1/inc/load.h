#ifndef _LOAD_H__
#define _LOAD_H__

#include <avr/io.h>

#define LOAD_PORT   PORTD
#define LOAD_DDR    DDRD
#define LOAD_BIT    DDD2

void load_init(void);
void load_connect(void);
void load_disconnect(void);

#endif

