#if !defined(_OS_H_)
#define _OS_H_

#include "lock.h"

#ifdef __cplusplus
extern "C"
{
#endif

  extern void os_presleep(void);
  extern void os_postsleep(void);
  void os_sleep(void);

  void os_init(void);
  void os_processTasks(void);
#ifdef __cplusplus
}
#endif

#endif // _OS_H_