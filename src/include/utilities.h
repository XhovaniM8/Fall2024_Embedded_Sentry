#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <array>
#include "include/system_config.h"

/**
 * Trims unnecessary or excess gyroscope data.
 * @param data A reference to the gyroscope data vector (3-axis).
 */
void trim_gyro_data(std::vector<std::array<float, 3>> &data);

/**
 * Calculates the correlation vectors between the saved gesture key and an unlocking record.
 * @param gesture_key The saved key as a vector of 3-axis gyroscope data.
 * @param unlocking_record The attempted unlocking gesture data.
 * @return A 3-element array containing the correlation for each axis (x, y, z).
 */
std::array<float, 3> calculateCorrelationVectors(
    const std::vector<std::array<float, 3>> &gesture_key,
    const std::vector<std::array<float, 3>> &unlocking_record);

#endif // UTILITIES_H