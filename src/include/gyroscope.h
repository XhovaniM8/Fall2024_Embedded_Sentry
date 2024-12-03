#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include <mbed.h>
#include <vector>
#include <array>
#include "system_config.h"

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
