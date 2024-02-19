#include "../../include/drivers/onewire.h"

#include <avr/io.h>
#include <util/delay.h>

#define _high_cycles 4
#define _sample_cycles 4
#define _low_cycles 7

// We need the 1.75 factor as the delay will be to short otherwise
#define usToCycles(us) ceil(fabs(((F_CPU / 1000000) * us * 1.1)))
//#define usToCycles(us) ceil(fabs(((3333333 / 1000000) * us * 1.1)))
//#define usToCycles(us) ((unsigned long)(ceil(fabs(((F_CPU / 1000000) * us )))))

//#define usToCycles(us) ceil(fabs(((F_CPU / 1000000) * us)))
extern void __builtin_avr_delay_cycles(unsigned long);

void _low()
{
  // Set out low
  OW_PORT.DIRSET = 1 << OW_PIN;
  OW_PORT.OUTCLR = 1 << OW_PIN;
}

void _high()
{
  // Set input (preferably floating)
  OW_PORT.DIRCLR = 1 << OW_PIN;
}

uint8_t _sample()
{
  return (OW_PORT.IN & (1 << OW_PIN)) > 0;
}

uint8_t ow_reset()
{
  uint8_t data;
  _low();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_H));
  _high();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_I));
  data = _sample();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_J));
  return data;
}

void ow_write(uint8_t data)
{
  for (uint8_t bit = 0; bit < 8; bit++)
  {
    if (data & 0x01)
    {
      _low();
      __builtin_avr_delay_cycles(usToCycles(OW_TIME_A) - _high_cycles);
      _high();
      __builtin_avr_delay_cycles(usToCycles(OW_TIME_B));
    }
    else
    {
      _low();
      __builtin_avr_delay_cycles(usToCycles(OW_TIME_C) - _high_cycles);
      _high();
      __builtin_avr_delay_cycles(usToCycles(OW_TIME_D));
    }
    data >>= 1;
  }
}

uint8_t ow_readBit(void)
{
  uint8_t data;
  _low();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_A));
  _high();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_E));
  data = _sample();
  __builtin_avr_delay_cycles(usToCycles(OW_TIME_F));
  return data;
}

uint8_t ow_read(void)
{
  uint8_t data;
  for (uint8_t bit = 0; bit < 8; bit++)
  {
    data >>= 1;
    if (ow_readBit())
      data |= 0x80;
  }
  return data;
}