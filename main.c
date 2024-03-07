// #include <stdio.h>
// #include <avr/iotn1614.h>
/// @mainpage MFM Sensor Module
/// @brief Main file for the MFM Sensor Module
/// @details This application will initialize the MFM Sensor board and perform
/// measurements from the following sensors:
/// - Huba713 sensor
/// - DS18B20 temperature sensor
/// - Atlas Scientific EZO EC sensor
/// Measurements are started by the I2C master by sending a command 0x10 to the
/// MFM Sensor Module (I2C address 0x36) The measurements will be stored in a
/// data packet and can be requested by the I2C master by sending a command 0x11
/// The data packet will be sent back to the I2C master and is defined as below,
/// - Number of bytes in the data packet (uint8_t)
/// - Huba713 pressure (uint16_t )
/// - Huba713 temperature (float in degrees Celsius)
/// - DS18B20 temperature (float in degrees Celsius)
/// - Atlas Scientific EZO EC conductivity (uint8_t[4]; string holding uS/cm)
/// The data will be in little-endian format
///
/// An example of the data packet:
/// > 0E C1 0B E0 7A A4 41 00 00 AD 41 30 2E 30 30
///
/// - Number of data bytes: 14
/// - Pressure: 3009
/// - Temperature: 20.5599975585
/// - ds18b20_temperature: 21.625
/// - conductivity: 0.00
/// When the MFM Sensor Module is not performing measurements, it will be in
/// sleep mode to save power

#include "board/mfm_sensor_module.h"

#include <avr/io.h>
#include <avr/wdt.h>

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <util/atomic.h>

#include "drivers/zacwire.h"
#include "mcu/twi.h"
#include "mcu/util.h"
#include "os/os.h"
#include "perif/atlas_ezo_ec.h"
#include "perif/ds18b20.h"
#include "perif/huba713.h"
#include <string.h>

// Power control
#define PWR_DISABLE 0x00
#define PWR_ENABLE 0x01

// forward declarations
static void pwr_5vEnable(uint8_t enable);
static void pwr_3v3Enable(uint8_t enable);
static void pwr_init(void);

void perform_measurements(void);

#define FLAG_CALIBRATED 0x01
#define HUBA_MEDIAN_COUNT 11

// Forward declaration of variables
/// @brief I2C Data packet
struct packet_t {
    uint16_t huba_pressure;
    float huba_temperature;
    float ds18b20_temperature;
    uint8_t atlas_conductivity[8];
    uint8_t flags;
} packet;

// "tasks" that will be performed on wakeup (interrupt)
volatile uint8_t doCalibration = 0;
volatile uint8_t doMeasurement = 0;

/// @brief DS18B20 struct to hold resolution, defined here so we can set it once
/// in main
ds18b20_t d;

/// @brief Initialize the power control
/// @details 5V and 3V3 will be disabled after initialization
void pwr_init(void) {
    ENABLE_5V_PORT.DIR |= ENABLE_5V_PIN;   // 5V_on as output
    ENABLE_3V3_PORT.DIR |= ENABLE_3V3_PIN; // 3V3_on as output

    pwr_3v3Enable(PWR_DISABLE);
    pwr_5vEnable(PWR_DISABLE);
}

/// @brief Enable/disable 5V
/// @param enable PWR_DISABLE (0x00) to disable, PWR_ENABLE (0x01) to enable
void pwr_5vEnable(uint8_t enable) {
    if (enable != 0x00) {
        ENABLE_5V_PORT.OUT |= ENABLE_5V_PIN; // Switch 5V_on to on
    } else {
        ENABLE_5V_PORT.OUT &= ~ENABLE_5V_PIN; // Switch 5V_on to off
    }
}

/// @brief Enable/disable 3V3
/// @param enable PWR_DISABLE (0x00) to disable, PWR_ENABLE (0x01) to enable
void pwr_3v3Enable(uint8_t enable) {
    if (enable != 0x00) {
        ENABLE_3V3_PORT.OUT &= ~ENABLE_3V3_PIN; // Switch 3V3_on to on
    } else {
        ENABLE_3V3_PORT.OUT |= ENABLE_3V3_PIN; // Switch 3V3_on to off
    }
}

/* Function to sort an array using insertion sort*/
void insertion_sort_u16(uint16_t arr[], int n) {
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 1 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

/* Function to sort an array using insertion sort*/
void insertion_sort_f(float arr[], int n) {
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 1 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

/// @brief Perform measurements from different sensors and store them in global
/// variables
/// @details This function is called from the twi_perform_task, which is called
/// from the I2C ISR it will enable 5V and 3V3, initialize sensors, perform the
/// measurements and disable 5V and 3V3
void perform_measurements() {
    // Conductivity data (string holding uS/cm)
    static uint8_t conductivity[8] = {0};
    static float ds18b20_temperature;
    static uint16_t huba_pressure = 0;
    static float huba_temperature = 0.0;

    uint16_t median_pressure[HUBA_MEDIAN_COUNT] = {0};
    float median_temperature[HUBA_MEDIAN_COUNT] = {0};

    // Enable 5V and 3V3
    pwr_5vEnable(PWR_ENABLE);
    pwr_3v3Enable(PWR_ENABLE);
    delay_ms(10);

    // Measure huba sensor using median filter
    int errs = 0;
    for (uint8_t ix = 0; ix < HUBA_MEDIAN_COUNT; ix++) {
        // read value from Huba sensor (zacwire)
        int err = huba713_read(&huba_pressure, &huba_temperature);
        if (err == 0) {
            median_pressure[ix] = huba_pressure;
            median_temperature[ix] = huba_temperature;
        } else {
            // If the parity is not 0x00, the data is invalid, so set values to
            // invalid
            errs++;
            median_pressure[ix] = 0;
            median_temperature[ix] = 200.0f;
        }
    }
    // Make sure there are enough valid measurements to perform median
    if (errs < HUBA_MEDIAN_COUNT / 2) {
        insertion_sort_u16(median_pressure, HUBA_MEDIAN_COUNT);
        insertion_sort_f(median_temperature, HUBA_MEDIAN_COUNT);
        huba_pressure = median_pressure[HUBA_MEDIAN_COUNT / 2];
        huba_temperature = median_temperature[HUBA_MEDIAN_COUNT / 2];
    } else {
        huba_pressure = 0;
        huba_temperature = 200.0f;
    }
    delay_us(1000);

    // The HUBA sensor is the only sensor in need of 5V, so disable it after
    // reading
    pwr_5vEnable(PWR_DISABLE);

    // read value from DS18B20 sensor (one-wire)
    ds18b20_read(&d, 0);
    ds18b20_temperature = ds18b20_read(&d, 0);

    // Turn the Atlas Scientific EZO EC sensor on by setting the enable pin
    atlas_ezo_ec_enable();

    // We only want to read the value once, so disable continuous reading
    atlas_ezo_ec_disableContinuousReading();

    // Set temperature compensation
    if (ds18b20_temperature > -50 && ds18b20_temperature < 50) {
        atlas_ezo_ec_setTemperature((uint8_t)ds18b20_temperature);
    } else {
        atlas_ezo_ec_setTemperature(10);
    }

    if (doCalibration) {
        atlas_ezo_ec_calibrate();
        packet.flags |= FLAG_CALIBRATED;
        doCalibration = 0;
    }

    // read value from Atlas Scientific EZO EC sensor (UART)
    atlas_ezo_ec_requestValue(conductivity);

    // Small delay before turning off the sensor
    delay_us(200);
    // Turn the Atlas Scientific EZO EC sensor off by clearing the enable pin
    atlas_ezo_ec_disable();

    // Small delay to give the CPU time to read the last data from the UART
    delay_us(500);

    // All done, so disable 3V3
    pwr_3v3Enable(PWR_DISABLE);

    // Store the data in data struct
    packet.huba_pressure = huba_pressure;
    packet.huba_temperature = huba_temperature;
    packet.ds18b20_temperature = ds18b20_temperature;
    for (int ii = 0; ii < 8; ii++) {
        packet.atlas_conductivity[ii] = conductivity[ii];
    }
}

/// @brief Handler for cmd 0x80 from I2C master
/// @details Triggers a calibration
/// @param buf Pointer to the buffer to store the data in
/// @param len Length of the buffer
void twi_cmd_80_handler(uint8_t *buf, uint8_t len) { doCalibration = 1; }

/// @brief Handler for cmd 0x11 from I2C master
/// @details Copies the sensor data from the data packet to the bus; kept short
/// as possible!
/// @param buf Pointer to the buffer to store the data in
/// @param len Length of the buffer
void twi_cmd_11_handler(uint8_t *buf, uint8_t len) {
    buf[0] = sizeof(struct packet_t);
    memcpy(&buf[1], &packet, buf[0]);
    memset(&packet, 0, sizeof(struct packet_t));
}

/// @brief Handler for cmd 0x10 from I2C master
/// @details Creates a task to start measurements; kept short as possible!
/// @param buf Pointer to the buffer to store the data in, not used
/// @param len Length of the buffer, not used
void twi_cmd_10_handler(uint8_t *buf, uint8_t len) { doMeasurement = 0x01; }

/// @brief Accapted TWI (I2C) commands
twi_cmd_t twi_cmds[] = {
    {0x10, &twi_cmd_10_handler},
    {0x11, &twi_cmd_11_handler},
    {0x80, &twi_cmd_80_handler},
};

/// @brief Waits for the watchdog to sync
/// @details
void wdt_sync(void) {
    // Wait for data to synchronize between clock and wdt domain
    while ((WDT.STATUS & WDT_SYNCBUSY_bm) == WDT_SYNCBUSY_bm)
        ;
}

/// @brief Function called before entering sleep
/// @details Disables BOD to save power; Disable the watchdog timer
void os_presleep() {
    wdt_disable();
    wdt_sync();
}

/// @brief Function called after waking up from sleep
void os_postsleep(void) {
    wdt_enable(WDT_PERIOD_8KCLK_gc);
    wdt_sync();
}

/// @brief main function
int main() {
    doMeasurement = 0;

    os_init();

    // Initialize the delay system
    delay_init();

    // Initialize the power control
    pwr_init();

    PORTA.DIRSET = PIN1_bm;
    PORTA.OUTTGL = PIN1_bm;
    // Make sure the peripherals (sensors) are off
    // delay_ms(1000);

    // Set resolution of DS18B20 temperature sensor
    d.resolution = DS18B20_RES_12;

    // Initialize the HUBA sensor
    huba713_init();

    // Enable interrupts
    sei();

    // Initialize the TWI Interface (set address to 0x36)
    twi_init(0x36, 1);

    // Set BOD mode for sleep mode (Disabled to save power)
    _PROTECTED_WRITE(BOD_CTRLA, BOD_CTRLA & ~(BOD_SLEEP_gm));

    // Set the watchdog timer
    wdt_enable(WDT_PERIOD_8KCLK_gc);
    wdt_sync();

    // Main loop
    while (1) {
        // Check if we need to perform measurements
        if (doMeasurement == 0x01) {
            doMeasurement = 0x00;
            perform_measurements();
        }

        // Kick the watchdog
        wdt_reset();

        // Sleep the system, note: watchdog will be disabled during sleep
        os_sleep();
    }
}
