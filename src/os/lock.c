#include "os/lock.h"
#include <util/atomic.h>

volatile os_lock_t locks[os_lock_count] = {0};

void os_lock(os_lock_t l) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { locks[l] = 1; }
}

void os_unlock(os_lock_t l) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { locks[l] = 0; }
}

uint8_t os_hasLock(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (int i = 0; i < os_lock_count; i++) {
      if (locks[i])
        return 1;
    }
  }
  return 0;
}
