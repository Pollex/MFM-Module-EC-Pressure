add_avr_library(mod_drivers STATIC
  onewire.c
  zacwire.c
)
avr_target_link_libraries(mod_drivers mod_mcu)
