/**
 * @file system_config.h
 * @author Xhovani Mali (xxm202)
 * @brief Header file for system configuration settings in the embedded sentry project.
 * @version 0.1
 * @date 2024-12-15
 *
 * @copyright Copyright (c) 2024
 *
 * @group Members:
 * - Xhovani Mali (xxm202)
 * - Shruti Pangare (stp8232)
 * - Temira Koenig (trk8600)
 * - Shruti Tulshidas Pangare
 */

#include <mbed.h>
#include <vector>
#include <array>
#include <limits>
#include <cmath>
#include <cstdio>
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#define CTRL_REG_1 0x20  // control register 1
#define CTRL_REG_3 0x22  // control register 3
#define CTRL_REG_4 0x23  // control register 4

#define OUT_X_L 0x28  // X-axis angular rate data Low

// Output data rate selections and cutoff frequencies
#define ODR_200_CUTOFF_50 0x60

// Interrupt configurations
#define INT2_DRDY 0x08   // Data ready on DRDY/INT2 pin

// Fullscale selections
#define FULL_SCALE_245 0x00       // full scale 245 dps
#define FULL_SCALE_500 0x10       // full scale 500 dps
#define FULL_SCALE_2000 0x20      // full scale 2000 dps
#define FULL_SCALE_2000_ALT 0x30  // full scale 2000 dps

// Sensitivities in dps/digit
#define SENSITIVITY_245 0.00875f  // 245 dps typical sensitivity
#define SENSITIVITY_500 0.0175f   // 500 dps typical sensitivity
#define SENSITIVITY_2000 0.07f    // 2000 dps typical sensitivity

// Convert constants
#define MY_LEG 1               // put board on left leg 0.8m above ground
#define DEGREE_TO_RAD 0.0175f  // rad = dgree * (pi / 180)

#define POWERON 0x0f   // turn gyroscope

// Event flags for event handler ()
#define KEY_FLAG 1
#define UNLOCK_FLAG 2
#define ERASE_FLAG 4
#define DATA_READY_FLAG 8
// on board discovery button
#define USER_BUTTON PA_0

//LCD font size
#define FONT_SIZE 16

// the unlocking threshold, change this to a smaller value if you have trouble unlocking (has to be positive)
#define CORRELATION_THRESHOLD .70f

#endif  // SYSTEM_CONFIG_H
