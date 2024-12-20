/**
 * @file utilities.h
 * @author Xhovani Mali (xxm202)
 * @brief Header file for utility functions in the embedded sentry project.
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

#ifndef UTILITIES_H
#define UTILITIES_H

// Include system configuration header
#include "system_config.h"

#define WINDOW_SIZE 5  // The size of the moving average window

// Function declarations for utility functions

/**
 * @brief Calculate the correlation for each coordinate between two vectors
 * @param vec1: the first vector (contains 3D data)
 * @param vec2: the second vector (contains 3D data)
 * @return the correlation for each coordinate (x, y, z) as a vector of floats
 */
array<float, 3> calculateCorrelationVectors(vector<array<float, 3>> &vec1,
                                            vector<array<float, 3>> &vec2);

/**
 * @brief Calculate the correlation between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the correlation between the two vectors
 */
float correlation(const vector<float> &a, const vector<float> &b);

/**
 * @brief Trim the gyro data based on a threshold
 * @param data: the gyro data to trim
 */
void trim_gyro_data(vector<array<float, 3>> &data);

/**
 * @brief Calculate the Dynamic Time Warping (DTW) distance between two vectors
 * @param s: the first vector
 * @param t: the second vector
 * @return the DTW distance between the two vectors
 */
float dtw(const vector<array<float, 3>> &s, const vector<array<float, 3>> &t);

/**
 * @brief Calculate the Euclidean distance between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the Euclidean distance between the two vectors
 */
float euclidean_distance(const array<float, 3> &a, const array<float, 3> &b);

/**
 * @brief Read gyro data from flash memory
 * @param flash_address: the flash address to read from
 * @param data_size: the size of the data to read
 * @return a vector containing the gyro data read from flash
 */
vector<array<float, 3>> readGyroDataFromFlash(uint32_t flash_address,
                                              size_t data_size);

/**
 * @brief Store gyro data to flash memory
 * @param gesture_key: the gyro data to store
 * @param flash_address: the address in flash to store the data
 * @return true if data is stored successfully, false otherwise
 */
bool storeGyroDataToFlash(vector<array<float, 3>> &gesture_key,
                          uint32_t flash_address);

#endif  // UTILITIES_H
