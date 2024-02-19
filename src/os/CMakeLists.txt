set(MOD_OS_FILES
  lock.c
  os.c
)

add_avr_library(mod_os STATIC ${MOD_OS_FILES})
