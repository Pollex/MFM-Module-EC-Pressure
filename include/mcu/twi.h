#if !defined(_TWI_H_)
#define _TWI_H_

#include <stdint.h>

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

#ifndef TWI_CMD_COUNT
#warning "TWI_CMD_COUNT must be defined manually to use memory efficiently"
#define TWI_CMD_COUNT 2
#endif

typedef struct
{
  uint8_t cmd;
  void (*handler)(uint8_t *data, uint8_t len);
} twi_cmd_t;

extern twi_cmd_t twi_cmds[];

#ifdef __cplusplus
extern "C"
{
#endif

  void twi_init(uint8_t addr, uint8_t enable_gc);
  void twi_ack(void);
  void twi_nack(void);

#ifdef __cplusplus
}
#endif

#endif // _TWI_H_