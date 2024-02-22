//
// Created by Eric van Rijswick on 24/01/2024.
//
#include <util/delay.h>
#include <avr/io.h>

#ifndef ZACWIRE_H
#define ZACWIRE_H


void zacwire_init(void);
int8_t zacwire_read(uint8_t *data, uint8_t count);

#endif // ZACWIRE_H
