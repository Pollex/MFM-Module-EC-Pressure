#if !defined(_DRIVER_ONEWIRE_H_)
#define _DRIVER_ONEWIRE_H_

#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 3333333UL
#endif

#ifndef OW_PORT
#define OW_PORT PORTA
#endif

#ifndef OW_PIN
#define OW_PIN PIN7
#endif

// Standard OneWire speeds in microseconds
// Duration of pull down before writing/reading a bit
#define OW_TIME_A 6
// Duration of holding 1/0 bit (high/low)
#define OW_TIME_B 64
// Duration of writing zero
#define OW_TIME_C 60
// Duration to wait after write/read to allow recharge
#define OW_TIME_D 10
// Duration after start to sample line
#define OW_TIME_E 9
// Duration to wait after sample
#define OW_TIME_F 55
// Only used in overdrive mode
#define OW_TIME_G 0
// Duration of reset pulldown
#define OW_TIME_H 480
// Duration after reset to sample presence
#define OW_TIME_I 70
// Duration after sample to wait
#define OW_TIME_J 410

// OneWire ROM Commands
#define OW_CMD_SKIP 0xCC
#define OW_CMD_READ 0x33
#define OW_CMD_SEARCH 0xF0
#define OW_CMD_MATCH 0x55

#ifdef __cplusplus
extern "C"
{
#endif

  uint8_t ow_reset(void);
  void ow_write(uint8_t);
  uint8_t ow_readBit(void);
  uint8_t ow_read(void);

#ifdef __cplusplus
}
#endif

#endif // _DRIVER_ONEWIRE_H_