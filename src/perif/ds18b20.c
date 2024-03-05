#include "../../include/perif/ds18b20.h"
#include "../../include/drivers/onewire.h"
#include "../../include/mcu/util.h"

#define ERROR_VALUE 100
#define MAX_RETRIES 5

void convert_t(uint8_t id) {
  ow_reset();
  ow_write(OW_CMD_SKIP);
  ow_write(DS18B20_CONVERT);
}

uint16_t read_temp() {
  ow_reset();
  ow_write(OW_CMD_SKIP);
  ow_write(DS18B20_READ_SCRATCHPAD);

  uint8_t lsb = ow_read();
  uint8_t msb = ow_read();

  return ((uint16_t)msb << 8) | (uint16_t)lsb;
}

void set_resolution(uint8_t res) {
  ow_reset();
  ow_write(OW_CMD_SKIP);
  ow_write(DS18B20_WRITE_SCRATCHPAD);
  ow_write(0x00);
  ow_write(0x00);
  ow_write(res << 5);
}

float get_res_bit(uint8_t res) {
  switch (res) {
  case DS18B20_RES_12:
    return 0.0625;
  case DS18B20_RES_11:
    return 0.125;
  case DS18B20_RES_10:
    return 0.25;
  case DS18B20_RES_9:
    return 0.5;
  }
  return 0.0625;
}

void wait_convert(uint8_t res) {
  switch (res) {
  case DS18B20_RES_12:
    delay_ms(750);
    break;
  case DS18B20_RES_11:
    delay_ms(375);
    break;
  case DS18B20_RES_10:
    delay_ms(188);
    break;
  case DS18B20_RES_9:
    delay_ms(94);
    break;
  }
}

float ds18b20_read(ds18b20_t *d, uint8_t id) {
  set_resolution(d->resolution);
  //  Read a new tempetarutre which is set in the scratchpad
  convert_t(id);

  wait_convert(d->resolution);
  // delay_ms(750);

  uint32_t start = millis();
  uint32_t now = start;
  do {
    delay_ms(20);
    now = millis();
    if (now - start > 1000)
      return ERROR_VALUE;
  } while (!ow_readBit());

  // Read scratchpad to get temperature bytes
  // sometimes read_temp returns 0xffff, so we retry up to `MAX_RETRIES` times
  uint8_t retries = 0;
  uint16_t raw;
  do {
    raw = read_temp();
    retries++;
    if (retries > MAX_RETRIES) {
      return ERROR_VALUE;
    }
  } while (raw == 0xffff);

  // Convert
  uint8_t sign = raw >> 15;
  float temp = (float)(raw & 0x7FFF) * get_res_bit(d->resolution);
  return temp * (sign ? -1 : 1);
}
