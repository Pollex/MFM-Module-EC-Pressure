//
// Created by Eric van Rijswick on 31/01/2024.
//

/// @file mfm_sensor_module.h
/// @brief Holds parameters for MFM Sensor Module board

#ifndef MFM_SENSOR_MODULE_MFM_SENSOR_MODULE_H
#define MFM_SENSOR_MODULE_MFM_SENSOR_MODULE_H

#include <avr/io.h>

#define USART_PORT PORTB
#define USART_RX_PIN PIN3_bm
#define USART_TX_PIN PIN2_bm

#define USART_ALTERNATE_PORT PORTA
#define USART_RX_ALTERNATE_PIN PIN2_bm
#define USART_TX_ALTERNATE_PIN PIN1_bm

#define USART_DEFAULT_PINS 0x00
#define USART_ALTERNATE_PINS 0x01

#define ENABLE_CONDUCTIVITY_PORT PORTA
#define ENABLE_CONDUCTIVITY_PIN PIN3_bm

#define ENABLE_5V_PORT PORTA
#define ENABLE_5V_PIN PIN4_bm
#define ENABLE_3V3_PORT PORTA
#define ENABLE_3V3_PIN PIN5_bm

#define ZACWIRE_PORT PORTA
#define ZACWIRE_PIN PIN6_bm
#define ZACWIRE_PINp PIN6_bp

#define TEST_PORT PORTA
#define TEST_PIN PIN2_bm

#endif //MFM_SENSOR_MODULE_MFM_SENSOR_MODULE_H
