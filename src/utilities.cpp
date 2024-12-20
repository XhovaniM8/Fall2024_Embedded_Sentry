/**
 * @file utilities.cpp
 * @author Xhovani Mali (xxm202)
 * @brief Utility functions implementation for the embedded sentry project.
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

#include "utilities.h"
#include <array>

array<float, 3> calculateCorrelationVectors(vector<array<float, 3>>& vec1, vector<array<float, 3>>& vec2) {
    array<float, 3> result;

    // Debug: Print sizes before checking for equality
    printf("Size of vec1: %zu, Size of vec2: %zu\n", vec1.size(), vec2.size());

    // Ensure both vectors are of the same size
    if (vec1.size() != vec2.size()) {
        printf("Error: Vectors have different sizes before resizing!\n");

        // Optionally, trim both vectors to the minimum size
        size_t min_size = std::min(vec1.size(), vec2.size());
        vec1.resize(min_size);
        vec2.resize(min_size);

        printf("Size of vec1 after resizing: %zu, Size of vec2 after resizing: %zu\n", vec1.size(), vec2.size());
    }

    size_t min_size = std::min(vec1.size(), vec2.size()); // Ensure both vectors are of equal size

    // Check after resizing
    if (vec1.size() != vec2.size()) {
        printf("Error: Vectors have different sizes after resizing!\n");
        return result;
    }

    for (int i = 0; i < 3; i++) {
        vector<float> a, b;
        for (size_t j = 0; j < min_size; ++j) {
            a.push_back(vec1[j][i]);
            b.push_back(vec2[j][i]);
        }

        result[i] = correlation(a, b);
    }

    return result;
}

/*******************************************************************************
 *
 * @brief Calculate the correlation between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the correlation between the two vectors
 *
 * ****************************************************************************/
float correlation(const vector<float> &a, const vector<float> &b) {
    // Check if the size of the two vectors are the same
    if (a.size() != b.size()) {
        printf("Incorrect Dimensions\n\r");
        return std::numeric_limits<float>::quiet_NaN(); // Return NaN as an error code
    }

    // Check if vectors are too small or have too much zero data
    bool has_variation = false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != 0.0f || b[i] != 0.0f) {
            has_variation = true;
            break;
        }
    }
    if (!has_variation) {
        printf("Insufficient variation in data.\n");
        return std::numeric_limits<float>::quiet_NaN(); // Return NaN for insufficient data
    }

    float sum_a = 0, sum_b = 0, sum_ab = 0, sq_sum_a = 0, sq_sum_b = 0;

    // Use Kahan summation for more stable summation
    for (size_t i = 0; i < a.size(); ++i) {
        float delta_a = a[i] - sum_a;
        float delta_b = b[i] - sum_b;
        sum_a += delta_a;
        sum_b += delta_b;
        sum_ab += delta_a * delta_b;
        sq_sum_a += a[i] * a[i];
        sq_sum_b += b[i] * b[i];
    }

    size_t n = a.size(); // Number of elements

    float numerator = sum_ab - (sum_a * sum_b / n);  // Covariance
    float denominator = sqrt((sq_sum_a - sum_a * sum_a / n) * (sq_sum_b - sum_b * sum_b / n));  // Standard deviation

    // Handle division by zero
    if (denominator == 0.0f) {
        return std::numeric_limits<float>::quiet_NaN(); // Avoid divide by zero
    }

    return numerator / denominator;
}

/*******************************************************************************
 *
 * @brief Trim the gyro data
 * @param data: the gyro data to trim
 *
 * ****************************************************************************/
void trim_gyro_data(vector<array<float, 3>> &data)
{
    float threshold = 0.00001;
    auto ptr = data.begin();

    // find the first element where data from any one direction is larger than the threshold
    while (abs((*ptr)[0]) <= threshold && abs((*ptr)[1]) <= threshold && abs((*ptr)[2]) <= threshold) {
        ptr++;
    }
    if (ptr == data.end())
        return;  // all data less than threshold

    auto lptr = ptr; // record the left bound
    // start searching from end to front
    ptr = data.end() - 1;
    while (abs((*ptr)[0]) <= threshold && abs((*ptr)[1]) <= threshold && abs((*ptr)[2]) <= threshold) {
        ptr--;
    }
    auto rptr = ptr; // record the right bound
    // start moving elements to the front
    auto replace_ptr = data.begin();
    for (; replace_ptr != lptr && lptr <= rptr; replace_ptr++, lptr++) {
        *replace_ptr = *lptr;
    }
    // trim the end
    if (lptr > rptr) {
        data.erase(replace_ptr, data.end());
    } else {
        data.erase(rptr + 1, data.end());
    }

    // Debug: Print the sizes of both vectors after trimming
    printf("Data after trimming, size: %zu\n", data.size());
}

/*******************************************************************************
 *
 * @brief Calculate the DTW distance between two vectors
 * @param s: the first vectorS
 * @param t: the second vector
 * @return the DTW distance between the two vectors
 *
 * ****************************************************************************/
float dtw(const vector<array<float, 3>> &s, const vector<array<float, 3>> &t)
{
    vector<vector<float>> dtw_matrix(s.size() + 1, vector<float>(t.size() + 1, numeric_limits<float>::infinity()));

    dtw_matrix[0][0] = 0;

    for (size_t i = 1; i <= s.size(); ++i)
    {
        for (size_t j = 1; j <= t.size(); ++j)
        {
            float cost = euclidean_distance(s[i - 1], t[j - 1]);
            dtw_matrix[i][j] = cost + min({dtw_matrix[i - 1][j], dtw_matrix[i][j - 1], dtw_matrix[i - 1][j - 1]});
        }
    }

    return dtw_matrix[s.size()][t.size()];
}

/*******************************************************************************
 *
 * @brief Calculate the euclidean distance between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the euclidean distance between the two vectors
 *
 * ****************************************************************************/
float euclidean_distance(const array<float, 3> &a, const array<float, 3> &b)
{
    float sum = 0;
    for (size_t i = 0; i < 3; ++i)
    {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

/*******************************************************************************
 *
 * @brief store data to flash
 * @param gesture_key: the data to store
 * @param flash_address: the address of the flash to store
 * @return true if the data is stored successfully, false otherwise
 *
 * ****************************************************************************/
bool storeGyroDataToFlash(vector<array<float, 3>> &gesture_key, uint32_t flash_address)
{
    FlashIAP flash;
    flash.init();

    // Calculate the total size of the data to be stored in bytes
    uint32_t data_size = gesture_key.size() * sizeof(array<float, 3>);

    // Erase the flash sector
    flash.erase(flash_address, data_size);

    // Write the data to flash
    int write_result = flash.program(gesture_key.data(), flash_address, data_size);

    flash.deinit();

    return write_result == 0;
}

/*******************************************************************************
 *
 * @brief read data from flash
 * @param flash_address: the address of the flash to read
 * @param data_size: the size of the data to read in bytes
 * @return a vector of array<float, 3> containing the data
 *
 * ****************************************************************************/
vector<array<float, 3>> readGyroDataFromFlash(uint32_t flash_address, size_t data_size)
{
    vector<array<float, 3>> gesture_key(data_size);

    FlashIAP flash;
    flash.init();

    // Read the data from flash
    flash.read(gesture_key.data(), flash_address, data_size * sizeof(array<float, 3>));

    flash.deinit();

    return gesture_key;
}




#include "utilities.h"
#include <array>

array<float, 3> calculateCorrelationVectors(vector<array<float, 3>>& vec1, vector<array<float, 3>>& vec2) {
    array<float, 3> result;

    // Debug: Print sizes before checking for equality
    printf("Size of vec1: %zu, Size of vec2: %zu\n", vec1.size(), vec2.size());

    // Ensure both vectors are of the same size
    if (vec1.size() != vec2.size()) {
        printf("Error: Vectors have different sizes before resizing!\n");

        // Optionally, trim both vectors to the minimum size
        size_t min_size = std::min(vec1.size(), vec2.size());
        vec1.resize(min_size);
        vec2.resize(min_size);

        printf("Size of vec1 after resizing: %zu, Size of vec2 after resizing: %zu\n", vec1.size(), vec2.size());
    }

    size_t min_size = std::min(vec1.size(), vec2.size()); // Ensure both vectors are of equal size

    // Check after resizing
    if (vec1.size() != vec2.size()) {
        printf("Error: Vectors have different sizes after resizing!\n");
        return result;
    }

    for (int i = 0; i < 3; i++) {
        vector<float> a, b;
        for (size_t j = 0; j < min_size; ++j) {
            a.push_back(vec1[j][i]);
            b.push_back(vec2[j][i]);
        }

        result[i] = correlation(a, b);
    }

    return result;
}


/*******************************************************************************
 *
 * @brief Calculate the correlation between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the correlation between the two vectors
 *
 * ****************************************************************************/
float correlation(const vector<float> &a, const vector<float> &b) {
    // Check if the size of the two vectors are the same
    if (a.size() != b.size()) {
        printf("Incorrect Dimensions\n\r");
        return std::numeric_limits<float>::quiet_NaN(); // Return NaN as an error code
    }

    // Check if vectors are too small or have too much zero data
    bool has_variation = false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != 0.0f || b[i] != 0.0f) {
            has_variation = true;
            break;
        }
    }
    if (!has_variation) {
        printf("Insufficient variation in data.\n");
        return std::numeric_limits<float>::quiet_NaN(); // Return NaN for insufficient data
    }

    float sum_a = 0, sum_b = 0, sum_ab = 0, sq_sum_a = 0, sq_sum_b = 0;

    // Use Kahan summation for more stable summation
    for (size_t i = 0; i < a.size(); ++i) {
        float delta_a = a[i] - sum_a;
        float delta_b = b[i] - sum_b;
        sum_a += delta_a;
        sum_b += delta_b;
        sum_ab += delta_a * delta_b;
        sq_sum_a += a[i] * a[i];
        sq_sum_b += b[i] * b[i];
    }

    size_t n = a.size(); // Number of elements

    float numerator = sum_ab - (sum_a * sum_b / n);  // Covariance
    float denominator = sqrt((sq_sum_a - sum_a * sum_a / n) * (sq_sum_b - sum_b * sum_b / n));  // Standard deviation

    // Handle division by zero
    if (denominator == 0.0f) {
        return std::numeric_limits<float>::quiet_NaN(); // Avoid divide by zero
    }

    return numerator / denominator;
}



/*******************************************************************************
 *
 * @brief Trim the gyro data
 * @param data: the gyro data to trim
 *
 * ****************************************************************************/
void trim_gyro_data(vector<array<float, 3>> &data)
{
    float threshold = 0.00001;
    auto ptr = data.begin();

    // find the first element where data from any one direction is larger than the threshold
    while (abs((*ptr)[0]) <= threshold && abs((*ptr)[1]) <= threshold && abs((*ptr)[2]) <= threshold) {
        ptr++;
    }
    if (ptr == data.end())
        return;  // all data less than threshold

    auto lptr = ptr; // record the left bound
    // start searching from end to front
    ptr = data.end() - 1;
    while (abs((*ptr)[0]) <= threshold && abs((*ptr)[1]) <= threshold && abs((*ptr)[2]) <= threshold) {
        ptr--;
    }
    auto rptr = ptr; // record the right bound
    // start moving elements to the front
    auto replace_ptr = data.begin();
    for (; replace_ptr != lptr && lptr <= rptr; replace_ptr++, lptr++) {
        *replace_ptr = *lptr;
    }
    // trim the end
    if (lptr > rptr) {
        data.erase(replace_ptr, data.end());
    } else {
        data.erase(rptr + 1, data.end());
    }

    // Debug: Print the sizes of both vectors after trimming
    printf("Data after trimming, size: %zu\n", data.size());
}

/*******************************************************************************
 *
 * @brief Calculate the DTW distance between two vectors
 * @param s: the first vectorS
 * @param t: the second vector
 * @return the DTW distance between the two vectors
 *
 * ****************************************************************************/
float dtw(const vector<array<float, 3>> &s, const vector<array<float, 3>> &t)
{
    vector<vector<float>> dtw_matrix(s.size() + 1, vector<float>(t.size() + 1, numeric_limits<float>::infinity()));

    dtw_matrix[0][0] = 0;

    for (size_t i = 1; i <= s.size(); ++i)
    {
        for (size_t j = 1; j <= t.size(); ++j)
        {
            float cost = euclidean_distance(s[i - 1], t[j - 1]);
            dtw_matrix[i][j] = cost + min({dtw_matrix[i - 1][j], dtw_matrix[i][j - 1], dtw_matrix[i - 1][j - 1]});
        }
    }

    return dtw_matrix[s.size()][t.size()];
}

/*******************************************************************************
 *
 * @brief Calculate the euclidean distance between two vectors
 * @param a: the first vector
 * @param b: the second vector
 * @return the euclidean distance between the two vectors
 *
 * ****************************************************************************/
float euclidean_distance(const array<float, 3> &a, const array<float, 3> &b)
{
    float sum = 0;
    for (size_t i = 0; i < 3; ++i)
    {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sqrt(sum);
}

/*******************************************************************************
 *
 * @brief store data to flash
 * @param gesture_key: the data to store
 * @param flash_address: the address of the flash to store
 * @return true if the data is stored successfully, false otherwise
 *
 * ****************************************************************************/
bool storeGyroDataToFlash(vector<array<float, 3>> &gesture_key, uint32_t flash_address)
{
    FlashIAP flash;
    flash.init();

    // Calculate the total size of the data to be stored in bytes
    uint32_t data_size = gesture_key.size() * sizeof(array<float, 3>);

    // Erase the flash sector
    flash.erase(flash_address, data_size);

    // Write the data to flash
    int write_result = flash.program(gesture_key.data(), flash_address, data_size);

    flash.deinit();

    return write_result == 0;
}

/*******************************************************************************
 *
 * @brief read data from flash
 * @param flash_address: the address of the flash to read
 * @param data_size: the size of the data to read in bytes
 * @return a vector of array<float, 3> containing the data
 *
 * ****************************************************************************/
vector<array<float, 3>> readGyroDataFromFlash(uint32_t flash_address, size_t data_size)
{
    vector<array<float, 3>> gesture_key(data_size);

    FlashIAP flash;
    flash.init();

    // Read the data from flash
    flash.read(gesture_key.data(), flash_address, data_size * sizeof(array<float, 3>));

    flash.deinit();

    return gesture_key;
}

