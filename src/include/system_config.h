#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <mbed.h>
#include <vector>
#include <array>
#include <limits>
#include <cmath>
#include <math.h>
#include "lib/drivers/LCD_DISCO_F429ZI.h"
#include "lib/drivers/TS_DISCO_F429ZI.h"

extern volatile uint32_t flags;

/**
 * Register Addresses for Gyroscope Control and Data
 */
#define WHO_AM_I 0x0F      // Device identification register
#define CTRL_REG_1 0x20    // Control register 1 (Output data rate and bandwidth)
#define CTRL_REG_2 0x21    // Control register 2
#define CTRL_REG_3 0x22    // Control register 3 (Interrupt settings)
#define CTRL_REG_4 0x23    // Control register 4 (Full scale selection)
#define CTRL_REG_5 0x24    // Control register 5
#define STATUS_REG 0x27    // Status register (Data-ready flags)
#define OUT_X_L 0x28       // X-axis angular rate low byte
#define OUT_X_H 0x29       // X-axis angular rate high byte
#define OUT_Y_L 0x2A       // Y-axis angular rate low byte
#define OUT_Y_H 0x2B       // Y-axis angular rate high byte
#define OUT_Z_L 0x2C       // Z-axis angular rate low byte
#define OUT_Z_H 0x2D       // Z-axis angular rate high byte
#define FIFO_CTRL_REG 0x2E // FIFO control register
#define FIFO_SRC_REG 0x2F  // FIFO status register

/**
 * Interrupt Configuration Registers
 */
#define INT1_CFG 0x30      // Interrupt 1 configuration register
#define INT1_SRC 0x31      // Interrupt 1 source register
#define INT1_TSH_XH 0x32   // Interrupt 1 threshold X-axis high byte
#define INT1_TSH_XL 0x33   // Interrupt 1 threshold X-axis low byte
#define INT1_TSH_YH 0x34   // Interrupt 1 threshold Y-axis high byte
#define INT1_TSH_YL 0x35   // Interrupt 1 threshold Y-axis low byte
#define INT1_TSH_ZH 0x36   // Interrupt 1 threshold Z-axis high byte
#define INT1_TSH_ZL 0x37   // Interrupt 1 threshold Z-axis low byte
#define INT1_DURATION 0x38 // Interrupt 1 duration register

/**
 * Output Data Rate (ODR) and Bandwidth Configurations
 */
#define ODR_100_CUTOFF_12_5 0x00
#define ODR_100_CUTOFF_25 0x10
#define ODR_200_CUTOFF_12_5 0x40
#define ODR_200_CUTOFF_25 0x50
#define ODR_200_CUTOFF_50 0x60
#define ODR_200_CUTOFF_70 0x70
#define ODR_400_CUTOFF_20 0x80
#define ODR_400_CUTOFF_25 0x90
#define ODR_400_CUTOFF_50 0xA0
#define ODR_400_CUTOFF_110 0xB0
#define ODR_800_CUTOFF_30 0xC0
#define ODR_800_CUTOFF_35 0xD0
#define ODR_800_CUTOFF_50 0xE0
#define ODR_800_CUTOFF_110 0xF0

/**
 * Interrupt Settings
 */
#define INT1_ENB 0x80
#define INT1_BOOT 0x40
#define INT1_ACT 0x20
#define INT1_OPEN 0x10
#define INT1_LATCH 0x02
#define INT1_ZHIE 0x20
#define INT1_ZLIE 0x10
#define INT1_YHIE 0x08
#define INT1_YLIE 0x04
#define INT1_XHIE 0x02
#define INT1_XLIE 0x01
#define INT2_DRDY 0x08

/**
 * Full-Scale Selections
 */
#define FULL_SCALE_245 0x00
#define FULL_SCALE_500 0x10
#define FULL_SCALE_2000 0x20
#define FULL_SCALE_2000_ALT 0x30

/**
 * Sensitivities (in dps/digit)
 */
#define SENSITIVITY_245 0.00875f
#define SENSITIVITY_500 0.0175f
#define SENSITIVITY_2000 0.07f

/**
 * Conversion Constants
 */
#define DEGREE_TO_RAD 0.0175f
#define MY_LEG 1 // Approximate vertical height (meters)

/**
 * Power States
 */
#define POWERON 0x0F
#define POWEROFF 0x00

/**
 * Timing Parameters
 */
#define SAMPLE_TIME_20 20
#define SAMPLE_INTERVAL_0_05 0.005f

/**
 * Event Flags for System State
 */
#define FLAG_KEY_RECORD (1U << 0)
#define FLAG_UNLOCK (1U << 1)
#define FLAG_ERASE (1U << 2)
#define FLAG_DATA_READY (1U << 3)

/**
 * System Constants
 */
#define LCD_FONT_SIZE 16
#define GESTURE_MAX_TIME 5000
#define GESTURE_MIN_TIME 500
#define SAMPLE_RATE_MS 50
#define CORRELATION_MIN 0.3f

/**
 * Hardware Pin Definitions
 */
#define USER_BUTTON PA_0
#define GYRO_INT_PIN PA_2
#define GYRO_CS_PIN PC_1
#define GYRO_MOSI_PIN PF_9
#define GYRO_MISO_PIN PF_8
#define GYRO_SCK_PIN PF_7

/**
 * Error Codes
 */
enum SystemError
{
    ERROR_NONE = 0,
    ERROR_GYRO_INIT = -1,
    ERROR_GESTURE_SHORT = -2,
    ERROR_GESTURE_INVALID = -3,
    ERROR_CALIBRATION = -4
};

#endif
