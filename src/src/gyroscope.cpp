#include <mbed.h>
#include <algorithm>
#include "gyroscope.h"

// SPI interface for the gyroscope
SPI gyroscope(PF_9, PF_8, PF_7); // MOSI, MISO, SCLK
DigitalOut cs(PC_1);             // Chip select

// Calibration thresholds for X, Y, Z axes
int16_t x_threshold = 0;
int16_t y_threshold = 0;
int16_t z_threshold = 0;

// Zero-rate level samples for X, Y, Z axes
int16_t x_sample = 0;
int16_t y_sample = 0;
int16_t z_sample = 0;

// Gyroscope sensitivity
float sensitivity = 0.0f;

// Pointer to store raw gyroscope data
Gyroscope_RawData *gyro_raw;

/**
 * @brief Writes a byte to a specified gyroscope register.
 *
 * @param address The register address.
 * @param data The data byte to write.
 */
void WriteByte(uint8_t address, uint8_t data)
{
    cs = 0; // Select the gyroscope
    gyroscope.write(address);
    gyroscope.write(data);
    cs = 1; // Deselect the gyroscope
}

/**
 * @brief Reads raw data from the gyroscope.
 *
 * @param rawdata Pointer to store the X, Y, Z raw data.
 */
void GetGyroValue(Gyroscope_RawData *rawdata)
{
    cs = 0;                                 // Select the gyroscope
    gyroscope.write(OUT_X_L | 0x80 | 0x40); // Auto-incremented read command
    rawdata->x_raw = gyroscope.write(0xFF) | (gyroscope.write(0xFF) << 8);
    rawdata->y_raw = gyroscope.write(0xFF) | (gyroscope.write(0xFF) << 8);
    rawdata->z_raw = gyroscope.write(0xFF) | (gyroscope.write(0xFF) << 8);
    cs = 1; // Deselect the gyroscope
}

/**
 * @brief Calibrates the gyroscope to determine zero-rate levels and thresholds.
 *
 * @param rawdata Pointer to store the raw gyroscope data during calibration.
 */
void CalibrateGyroscope(Gyroscope_RawData *rawdata)
{
    int16_t sumX = 0, sumY = 0, sumZ = 0;

    printf("========[Calibrating...]========\r\n");
    for (int i = 0; i < 128; i++)
    {
        GetGyroValue(rawdata);
        sumX += rawdata->x_raw;
        sumY += rawdata->y_raw;
        sumZ += rawdata->z_raw;

        x_threshold = std::max(x_threshold, static_cast<int16_t>(abs(rawdata->x_raw)));
        y_threshold = std::max(y_threshold, static_cast<int16_t>(abs(rawdata->y_raw)));
        z_threshold = std::max(z_threshold, static_cast<int16_t>(abs(rawdata->z_raw)));

        wait_us(10000); // 10ms delay between samples
    }

    // Calculate the average zero-rate levels
    x_sample = sumX >> 7; // Divide by 128 (2^7)
    y_sample = sumY >> 7;
    z_sample = sumZ >> 7;

    printf("========[Calibration Complete]========\r\n");
}

/**
 * @brief Initializes the gyroscope by configuring its control registers.
 *
 * @param init_parameters Gyroscope initialization parameters.
 * @param init_raw_data Pointer to store initial raw data.
 */
void InitiateGyroscope(Gyroscope_Init_Parameters *init_parameters, Gyroscope_RawData *init_raw_data)
{
    printf("\r\n========[Initializing Gyroscope...]========\r\n");
    gyro_raw = init_raw_data;

    cs = 1; // Ensure chip is deselected initially

    // Configure the gyroscope
    gyroscope.format(8, 3);       // 8-bit data frame, polarity 1, phase 0
    gyroscope.frequency(1000000); // Clock frequency: 1 MHz

    WriteByte(CTRL_REG_1, init_parameters->conf1 | POWERON); // Enable all axes and set data rate
    WriteByte(CTRL_REG_3, init_parameters->conf3);           // Enable DRDY interrupt
    WriteByte(CTRL_REG_4, init_parameters->conf4);           // Set full-scale selection

    // Set sensitivity based on full-scale selection
    switch (init_parameters->conf4)
    {
    case FULL_SCALE_245:
        sensitivity = SENSITIVITY_245;
        break;
    case FULL_SCALE_500:
        sensitivity = SENSITIVITY_500;
        break;
    case FULL_SCALE_2000:
    case FULL_SCALE_2000_ALT:
        sensitivity = SENSITIVITY_2000;
        break;
    default:
        printf("Invalid sensitivity configuration\r\n");
        return;
    }

    CalibrateGyroscope(gyro_raw); // Calibrate the gyroscope
    printf("========[Initialization Complete]========\r\n");
}

/**
 * @brief Converts raw data to angular velocity in degrees per second (dps).
 *
 * @param axis_data Raw axis data.
 * @return Angular velocity in dps.
 */
float ConvertToDPS(int16_t axis_data)
{
    return axis_data * sensitivity;
}

/**
 * @brief Converts angular velocity (dps) to linear velocity (m/s).
 *
 * @param axis_data Raw axis data.
 * @return Linear velocity in m/s.
 */
float ConvertToVelocity(int16_t axis_data)
{
    return axis_data * sensitivity * DEGREE_TO_RAD * MY_LEG;
}

/**
 * @brief Calculates total distance from raw gyroscope data.
 *
 * @param arr Array of raw gyroscope data.
 * @return Total distance in meters.
 */
float GetDistance(int16_t arr[])
{
    float distance = 0.0f;

    for (int i = 0; i < 400; i++)
    {
        float velocity = ConvertToVelocity(arr[i]);
        distance += abs(velocity * 0.05f); // Assume a sampling interval of 0.05s
    }

    return distance;
}

/**
 * @brief Processes raw gyroscope data to remove zero-rate offsets and apply thresholds.
 */
void GetCalibratedRawData()
{
    GetGyroValue(gyro_raw);

    // Remove zero-rate offsets
    gyro_raw->x_raw -= x_sample;
    gyro_raw->y_raw -= y_sample;
    gyro_raw->z_raw -= z_sample;

    // Apply thresholds to filter noise
    if (abs(gyro_raw->x_raw) < x_threshold)
        gyro_raw->x_raw = 0;
    if (abs(gyro_raw->y_raw) < y_threshold)
        gyro_raw->y_raw = 0;
    if (abs(gyro_raw->z_raw) < z_threshold)
        gyro_raw->z_raw = 0;
}

/**
 * @brief Powers off the gyroscope by disabling all axes.
 */
void PowerOff()
{
    WriteByte(CTRL_REG_1, POWEROFF);
}
