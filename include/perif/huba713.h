//
// Created by Eric van Rijswick on 29/01/2024.
//

#ifndef MFM_SENSOR_MODULE_HUBA713_H
#define MFM_SENSOR_MODULE_HUBA713_H
#include <stdint.h>
#include <avr/io.h>

uint8_t huba713_read(uint16_t *pressure, float *temperature);
void huba713_init(void);

#endif //MFM_SENSOR_MODULE_HUBA713_H
