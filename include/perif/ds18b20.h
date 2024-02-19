#if !defined(_PERIF_DS18B20_H_)
#define _PERIF_DS18B20_H_

#include <stdint.h>

#define DS18B20_CONVERT 0x44
#define DS18B20_WRITE_SCRATCHPAD 0x4E
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_COPY_SCRATCHPAD 0x48

#define DS18B20_RES_12 3
#define DS18B20_RES_11 2
#define DS18B20_RES_10 1
#define DS18B20_RES_9 0

typedef struct ds18b20_t {
   uint8_t resolution; 
} ds18b20_t;

#ifdef __cplusplus
extern "C"
{
#endif

  float ds18b20_read(ds18b20_t* d, uint8_t id);

#ifdef __cplusplus
}
#endif

#endif // _PERIF_DS18B20_H_
