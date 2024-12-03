#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include <mbed.h>
#include <vector>
#include <array>

// Register Addresses for Gyroscope Control and Data
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

// Interrupt Configuration Registers
#define INT1_CFG 0x30      // Interrupt 1 configuration register
#define INT1_SRC 0x31      // Interrupt 1 source register
#define INT1_TSH_XH 0x32   // Interrupt 1 threshold X-axis high byte
#define INT1_TSH_XL 0x33   // Interrupt 1 threshold X-axis low byte
#define INT1_TSH_YH 0x34   // Interrupt 1 threshold Y-axis high byte
#define INT1_TSH_YL 0x35   // Interrupt 1 threshold Y-axis low byte
#define INT1_TSH_ZH 0x36   // Interrupt 1 threshold Z-axis high byte
#define INT1_TSH_ZL 0x37   // Interrupt 1 threshold Z-axis low byte
#define INT1_DURATION 0x38 // Interrupt 1 duration register

// Output Data Rate (ODR) and Bandwidth Configurations
#define ODR_100_CUTOFF_12_5 0x00 // 100 Hz ODR, 12.5 Hz cutoff
#define ODR_100_CUTOFF_25 0x10   // 100 Hz ODR, 25 Hz cutoff
#define ODR_200_CUTOFF_12_5 0x40 // 200 Hz ODR, 12.5 Hz cutoff
#define ODR_200_CUTOFF_25 0x50   // 200 Hz ODR, 25 Hz cutoff
#define ODR_200_CUTOFF_50 0x60   // 200 Hz ODR, 50 Hz cutoff
#define ODR_200_CUTOFF_70 0x70   // 200 Hz ODR, 70 Hz cutoff
#define ODR_400_CUTOFF_20 0x80   // 400 Hz ODR, 20 Hz cutoff
#define ODR_400_CUTOFF_25 0x90   // 400 Hz ODR, 25 Hz cutoff
#define ODR_400_CUTOFF_50 0xA0   // 400 Hz ODR, 50 Hz cutoff
#define ODR_400_CUTOFF_110 0xB0  // 400 Hz ODR, 110 Hz cutoff
#define ODR_800_CUTOFF_30 0xC0   // 800 Hz ODR, 30 Hz cutoff
#define ODR_800_CUTOFF_35 0xD0   // 800 Hz ODR, 35 Hz cutoff
#define ODR_800_CUTOFF_50 0xE0   // 800 Hz ODR, 50 Hz cutoff
#define ODR_800_CUTOFF_110 0xF0  // 800 Hz ODR, 110 Hz cutoff

// High Pass Filter Configurations (Mode Disabled)
#define ODR_100_HIGH_PASS_8 0x00  // 100 Hz ODR, high-pass filter disabled
#define ODR_200_HIGH_PASS_15 0x00 // 200 Hz ODR, high-pass filter disabled
#define ODR_400_HIGH_PASS_30 0x00 // 400 Hz ODR, high-pass filter disabled
#define ODR_800_HIGH_PASS_56 0x00 // 800 Hz ODR, high-pass filter disabled

// Interrupt Settings
#define INT1_ENB 0x80   // Enable interrupt on INT1 pin
#define INT1_BOOT 0x40  // Boot status available on INT1 pin
#define INT1_ACT 0x20   // Active interrupt configuration on INT1 pin
#define INT1_OPEN 0x10  // INT1 pin configuration
#define INT1_LATCH 0x02 // Latch interrupt request in INT1_SRC register
#define INT1_ZHIE 0x20  // Enable interrupt for Z-axis high event
#define INT1_ZLIE 0x10  // Enable interrupt for Z-axis low event
#define INT1_YHIE 0x08  // Enable interrupt for Y-axis high event
#define INT1_YLIE 0x04  // Enable interrupt for Y-axis low event
#define INT1_XHIE 0x02  // Enable interrupt for X-axis high event
#define INT1_XLIE 0x01  // Enable interrupt for X-axis low event
#define INT2_DRDY 0x08  // Data-ready interrupt on DRDY/INT2 pin

// Full-Scale Selections
#define FULL_SCALE_245 0x00      // Full scale: ±245 dps
#define FULL_SCALE_500 0x10      // Full scale: ±500 dps
#define FULL_SCALE_2000 0x20     // Full scale: ±2000 dps
#define FULL_SCALE_2000_ALT 0x30 // Alternate selection: ±2000 dps

// Sensitivities (in dps/digit)
#define SENSITIVITY_245 0.00875f // Sensitivity for ±245 dps
#define SENSITIVITY_500 0.0175f  // Sensitivity for ±500 dps
#define SENSITIVITY_2000 0.07f   // Sensitivity for ±2000 dps

// Conversion Constants
#define DEGREE_TO_RAD 0.0175f // Radians = Degrees × (π / 180)
#define MY_LEG 1              // Approximate vertical height (meters) for velocity calculations

// Power States
#define POWERON 0x0F  // Turn gyroscope ON
#define POWEROFF 0x00 // Turn gyroscope OFF

// Timing Parameters
#define SAMPLE_TIME_20 20           // Sample time for 20 Hz
#define SAMPLE_INTERVAL_0_05 0.005f // Sample interval (seconds)

// Data Structures
typedef struct
{
    uint8_t conf1; // Output data rate configuration
    uint8_t conf3; // Interrupt configuration
    uint8_t conf4; // Full-scale selection
} Gyroscope_Init_Parameters;

typedef struct
{
    int16_t x_raw; // X-axis raw data
    int16_t y_raw; // Y-axis raw data
    int16_t z_raw; // Z-axis raw data
} Gyroscope_RawData;

typedef struct
{
    int16_t x_calibrated; // X-axis calibrated data
    int16_t y_calibrated; // Y-axis calibrated data
    int16_t z_calibrated; // Z-axis calibrated data
} Gyroscope_CalibratedData;

// Function Prototypes
void WriteByte(uint8_t address, uint8_t data);
void GetGyroValue(Gyroscope_RawData *rawdata);
void CalibrateGyroscope(Gyroscope_RawData *rawdata);
void InitiateGyroscope(Gyroscope_Init_Parameters *init_parameters, Gyroscope_RawData *init_raw_data);
float ConvertToDPS(int16_t rawdata);
float ConvertToVelocity(int16_t rawdata);
float GetDistance(int16_t arr[]);
void GetCalibratedRawData();
void PowerOff();

#endif // GYROSCOPE_H
