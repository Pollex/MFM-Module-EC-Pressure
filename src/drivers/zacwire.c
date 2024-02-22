#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "board/mfm_sensor_module.h"
#include "drivers/zacwire.h"

#define IDLE_COUNTS_MS                                                         \
    (F_CPU /                                                                   \
     1000) // F_CPU is counts per second, div by 1000 to get counts per ms
#define IDLE_COUNTS IDLE_COUNTS_MS / 2 // 0.5ms

volatile enum State {
    STATE_WAIT_FOR_IDLE,
    STATE_WAIT_FOR_START_BIT,
    STATE_WAIT_FOR_FIRST_BIT,
    STATE_WAIT_FOR_BIT,
    STATE_BIT_SAMPLING,
    STATE_DONE
} state;

volatile struct {
    uint8_t duty_cycle;
    uint8_t bit_count;
    uint8_t *data_ptr;
    uint8_t read_count;
    uint8_t parity;
} state_ctx;

void timer_set(uint16_t counts) {
    TCB0.CNT = 0;
    TCB0.CCMP = counts;
    TCB0.CTRLA = TCB_ENABLE_bm;
}

/// @brief Initialize the zacwire bus
void zacwire_init() {
    // Set the pin as input
    ZACWIRE_PORT.DIR &= ~ZACWIRE_PIN;
    //
    // Prepare timer
    //
    // TCB uses CLK_PER which is based on F_CPU so 3_333_333Hz in this case
    // One tick would be around 300ns
    // No clk div is used
    // TCB0.CTRLA = 0;
    // Set to single shot mode, and capture events when counter reaches TOP
    // require an interrupt, only the counted value
    TCB0.CTRLB = TCB_CNTMODE_SINGLE_gc;
}

/// @brief Read a byte from the bus
/// @details Read a byte from the (32kHz) bus
/// @param data pointer to the data byte
/// @return validity of the data: 0 is valid, -1 is invalid
/// @condition The bus is idle
int8_t zacwire_read(uint8_t *data, uint8_t count) {
    // Reset SM and enable interrupts
    state = STATE_WAIT_FOR_IDLE;
    state_ctx.parity = 0;
    state_ctx.bit_count = 0;
    state_ctx.duty_cycle = 0;
    state_ctx.data_ptr = data;
    state_ctx.read_count = count;
    ZACWIRE_PORT.PIN6CTRL |= PORT_ISC_FALLING_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    // Start idle timer, this will interrupt the timer after 0.5 ms, or
    // if a falling edge is detected, this timer will be reset
    timer_set(IDLE_COUNTS);

    // Wait for SM to finish
    while (state != STATE_DONE)
        ;

    // Disable interrupts
    ZACWIRE_PORT.PIN6CTRL &= ~PORT_ISC_FALLING_gc;
    TCB0.INTCTRL = 0;

    if (state_ctx.parity % 2) {
        return -1;
    }
    return 0;
}

void sm_on_falling_edge(void) {
    switch (state) {
    case STATE_WAIT_FOR_IDLE:
        // We reached a low edge, meaning a transaction is busy. Reset the
        // timer.
        timer_set(IDLE_COUNTS);
        break;
    case STATE_WAIT_FOR_START_BIT:
        // A falling edge while waiting for the start bit. start counter
        timer_set(0xffff);
        // At the next falling edge, store the counts
        state = STATE_WAIT_FOR_FIRST_BIT;
        break;
    case STATE_WAIT_FOR_FIRST_BIT:
        state_ctx.duty_cycle = TCB0.CNT /2;
        // fallthrough to bit start
    case STATE_WAIT_FOR_BIT:
        // Set the timer to the duty cycle sample point.
        timer_set(state_ctx.duty_cycle);
        // After the timer interrupts, it will sample the bit and either
        // move to BYTE_END or back to this state
        state = STATE_BIT_SAMPLING;
        break;

    // These states should not have falling edges
    case STATE_BIT_SAMPLING:
    case STATE_DONE:
        break;
    }
}

void sm_on_timer(void) {
    switch (state) {
    case STATE_WAIT_FOR_IDLE:
        // The timer is only triggered in this state if no falling edge is
        // detected during 1ms
        state = STATE_WAIT_FOR_START_BIT;
        break;
    case STATE_BIT_SAMPLING: {
        // Timer triggers at 1/2 Cycle so we should sample NOW!
        uint8_t sample = ((ZACWIRE_PORT.IN & ZACWIRE_PIN) > 0);
        state_ctx.parity += sample;
        state_ctx.bit_count++; // Increment first, so "1-based" counter
        // These are still data bits
        if (state_ctx.bit_count < 9) {
            *state_ctx.data_ptr <<= 1;
            *state_ctx.data_ptr |= sample;
            state = STATE_WAIT_FOR_BIT;
        } else {
            // We're at the 9th bit (parity bit)
            // If we require more bytes to be read, loop back to wait for start bit;
            if ((--state_ctx.read_count) > 0) {
                state_ctx.data_ptr++;
                state = STATE_WAIT_FOR_START_BIT;
            } else {
                // Otherwise, goto IDLE
                state = STATE_DONE;
            }
        }
        break;
    }

    // Timer should not trigger in these states!
    case STATE_WAIT_FOR_START_BIT:
    case STATE_WAIT_FOR_FIRST_BIT:
    case STATE_WAIT_FOR_BIT:
    case STATE_DONE:
        break;
    }
}

ISR(PORTA_PORT_vect) {
    PORTA.OUTTGL = PIN1_bm;
    //sm_on_falling_edge();
    ZACWIRE_PORT.INTFLAGS = ZACWIRE_PIN;
}

ISR(TCB0_INT_vect) {
    sm_on_timer();
    TCB0.INTFLAGS = 1;
}
