//
// Created by Eric van Rijswick on 31/01/2024.
//

#ifndef MFM_SENSOR_MODULE_ATLAS_EZO_EC_H
#define MFM_SENSOR_MODULE_ATLAS_EZO_EC_H

//void atlas_ezo_ec_init(void);
int atlas_ezo_ec_requestValue(char *value);
int atlas_ezo_ec_disableContinuousReading(void);
int atlas_ezo_ec_waitForBoot(void);
void atlas_ezo_ec_disable(void);
void atlas_ezo_ec_enable(void);

#endif //MFM_SENSOR_MODULE_ATLAS_EZO_EC_H
