#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "board/mfm_sensor_module.h"
#include "drivers/zacwire.h"

#define IDLE_COUNTS 150 // 272us

/// @brief Initialize the zacwire bus
void zacwire_init() {
    // Set the pin as input
    ZACWIRE_PORT.DIRCLR = ZACWIRE_PIN;
}

inline void wait_for_idle(void) {
    // Wait for bus idle
    uint16_t idle = IDLE_COUNTS;
    while (idle) {
        if (ZACWIRE_PORT.IN & ZACWIRE_PIN)
            idle--;
        else
            idle = IDLE_COUNTS;
    }
}

inline void wait_till_low(void) {
    while (!(ZACWIRE_PORT.IN & ZACWIRE_PIN))
        ;
    while (ZACWIRE_PORT.IN & ZACWIRE_PIN)
        ;
}

inline void wait_till_duty(uint8_t cycles) {
    while (cycles--)
        __asm("nop");
}

void zacwire_read_byte(uint8_t *data, uint8_t *parity) {
    uint8_t duty = 0;
    uint8_t bits = 8;

    // Measure half duty using start bit (not a data bit!)
    wait_till_low();
    while (!(ZACWIRE_PORT.IN & ZACWIRE_PIN))
        duty++;

    while (bits--) {
        wait_till_low();
        wait_till_duty(duty);
        PORTA.OUTTGL = PIN1_bm;
        // Sample
        *data <<= 1;
        if ((ZACWIRE_PORT.IN & ZACWIRE_PIN) > 0) {
            *data |= 1;
            *parity += 1;
        }
    }
    wait_till_low();
    wait_till_duty(duty);
    if ((ZACWIRE_PORT.IN & ZACWIRE_PIN) > 0)
        *parity += 1;
}

/// @brief Read a byte from the bus
/// @details Read a byte from the (32kHz) bus
/// @param data pointer to the data byte
/// @return validity of the data: 0 is valid, -1 is invalid
/// @condition The bus is idle
int8_t zacwire_read(uint8_t *data, uint8_t count) {
    uint8_t parity = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        wait_for_idle();
        while (count--) {
            zacwire_read_byte(data, &parity);
            data++;
        }
    }
    return -(parity % 2);
}
