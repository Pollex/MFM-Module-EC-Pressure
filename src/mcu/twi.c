#include "mcu/twi.h"

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "os/os.h"

uint8_t twi_buffer[TWI_BUFFER_LENGTH] = {0};
volatile uint8_t twi_buffer_rx = 0;
volatile uint8_t twi_buffer_tx = 0;
volatile uint8_t twi_busy = 0;
volatile twi_cmd_t *twi_current_cmd;

void twi_ack() { TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc; }
void twi_nack() { TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc | TWI_ACKACT_NACK_gc; } //RESPONSE, NACK
void twi_complete() { TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc; }                 //COMPTRANS
void twi_end()
{
  twi_complete();
  os_unlock(os_lock_twi);
  twi_busy = 0;
}

/// @brief Initialize the TWI interface
/// @param addr The address of the device
void twi_init(uint8_t addr, uint8_t enable_gc)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Enable interrupts
    TWI0.SCTRLA = TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm | TWI_SMEN_bm;
    // Set addr
    TWI0.SADDR = addr << 1 | enable_gc;
    // Enable
    TWI0.SCTRLA |= TWI_ENABLE_bm;
  }
}

ISR(TWI0_TWIS_vect)
{
  os_lock(os_lock_twi);
  uint8_t s = TWI0.SSTATUS;
  uint8_t isErr = s & (TWI_COLL_bm | TWI_BUSERR_bm);
  uint8_t isRead = (s & (TWI_DIF_bm | TWI_DIR_bm)) == 0x82;
  uint8_t isWrite = (s & (TWI_DIF_bm | TWI_DIR_bm)) == 0x80;
  uint8_t isAddr = (s & (TWI_APIF_bm | TWI_AP_bm)) == 0x41;
  uint8_t isStop = (s & (TWI_APIF_bm | TWI_AP_bm)) == 0x40;
  uint8_t rxnack = s & TWI_RXACK_bm;

  if (isErr)
  {
    return twi_end();
  }

  if (isStop)
  {
    if (twi_current_cmd)
      twi_current_cmd->handler(twi_buffer, twi_buffer_rx);
    return twi_end();
  }

  if (isAddr)
  {
    // Is restart, call handler
    if (twi_busy && twi_current_cmd)
      twi_current_cmd->handler(twi_buffer, twi_buffer_rx);

    twi_busy = 1;
    twi_buffer_rx = 0;
    twi_buffer_tx = 0;
    twi_current_cmd = 0;
    return twi_ack();
  }

  if (isWrite)
  {
    if (twi_buffer_rx >= TWI_BUFFER_LENGTH)
      return twi_nack();
    twi_buffer[twi_buffer_rx++] = TWI0.SDATA;

    // Find command handler
    if (twi_buffer_rx == 1)
    {
      for (uint8_t i = 0; i < TWI_CMD_COUNT; i++)
      {
        if (twi_cmds[i].cmd == twi_buffer[0])
        {
          twi_current_cmd = &twi_cmds[i];
          break;
        }
      }
    }

    // ACK if command is found
    if (twi_current_cmd != 0)
      return twi_ack();
    // NACK if command is not found
    return twi_nack();
  }

  if (isRead)
  {
    if ((twi_buffer_tx && rxnack) || twi_buffer_tx >= TWI_BUFFER_LENGTH)
      return twi_complete();

    TWI0.SDATA = twi_buffer[twi_buffer_tx++];
    return twi_ack();
  }

  twi_end();
}