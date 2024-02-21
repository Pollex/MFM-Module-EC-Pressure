## TODOs

### Open Q's
- [ ] Calibration must happens when probe is switched. When and how to do this?
- [ ] Is it worth disabling the EZO circuit LED?
  - Perhaps sending the command takes more power than the LED being on

### Atlas Scientific EZO Circuit
- [ ] Validate output string is always 4 chars
    - Create alrge enough ascii buffer and set to 0.0000 before reading ezo kit
- [ ] Send probe K value from EEPROM in perform_measurement
- [ ] Send calibration string from EEPROM in perform_measurement
- [ ] Send water temperature from EEPROM in perform_measurement

