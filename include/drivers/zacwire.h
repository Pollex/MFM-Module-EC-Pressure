//
// Created by Eric van Rijswick on 24/01/2024.
//
#include <util/delay.h>
#include <avr/io.h>

#ifndef ZACWIRE_H
#define ZACWIRE_H


void zacwire_init(void);
void zacwire_waitForIdle(void);
uint8_t zacwire_readByte(uint8_t *data);

#endif // ZACWIRE_H