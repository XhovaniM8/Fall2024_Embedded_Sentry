/*
MIT License

CopyRight (c) 2023 Charlie Wu.
Copyright (c) 2021 Haoran Wang.
Copyright (c) 2020 Steffen S.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <mbed.h>
#include "system_config.h"

// Initialization parameters
typedef struct
{
    uint8_t conf1;       // output data rate
    uint8_t conf3;       // interrupt configuration
    uint8_t conf4;       // full sacle selection
} Gyroscope_Init_Parameters;

// Raw data
typedef struct
{
    int16_t x_raw; // X-axis raw data
    int16_t y_raw; // Y-axis raw data
    int16_t z_raw; // Z-axis raw data
} Gyroscope_RawData;

// Calibrated data
typedef struct
{
    int16_t x_calibrated; // X-axis calibrated data
    int16_t y_calibrated; // Y-axis calibrated data
    int16_t z_calibrated; // Z-axis calibrated data
} Gyroscope_CalibratedData;

// Write IO
void WriteByte(uint8_t address, uint8_t data);

// Read IO
void GetGyroValue(Gyroscope_RawData *rawdata);

// Gyroscope calibration
void CalibrateGyroscope(Gyroscope_RawData *rawdata);

// Gyroscope initialization
void InitiateGyroscope(Gyroscope_Init_Parameters *init_parameters, Gyroscope_RawData *init_raw_data);

// Data conversion: raw -> dps
float ConvertToDPS(int16_t rawdata);

// Data conversion: dps -> m/s
float ConvertToVelocity(int16_t rawdata);

// Calculate distance from raw data array;
float GetDistance(int16_t arr[]);

// Get calibrated data
void GetCalibratedRawData();

// Turn off the gyroscope
void PowerOff();