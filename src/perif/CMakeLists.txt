add_avr_library(mod_perif STATIC
  ds18b20.c
  huba713.c
  atlas_ezo_ec.c
)
avr_target_link_libraries(mod_perif mod_drivers mod_mcu)
