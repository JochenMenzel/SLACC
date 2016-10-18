#include "load.h"
#include <avr/io.h>


void load_init(void)
{
    // set pins as output
    LOAD_DDR |= 1 << LOAD_BIT; // connect load
}


void load_connect(void)
{
    LOAD_PORT |= 1 << LOAD_BIT;
}


void load_disconnect(void)
{

    LOAD_PORT &= ~(1 << LOAD_BIT);
}

