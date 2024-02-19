#if !defined(_OS_LOCKS_H_)
#define _OS_LOCKS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum os_lock
  {
    os_lock_twi,
    os_lock_count
  } os_lock_t;

  void os_lock(os_lock_t l);
  void os_unlock(os_lock_t l);
  uint8_t os_hasLock(void);

#ifdef __cplusplus
}
#endif

#endif // _OS_LOCKS_H_