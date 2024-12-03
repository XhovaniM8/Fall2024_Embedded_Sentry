#include "utilities.h"
#include "system_config.h"
#include <cmath>
#include <numeric>
#include <algorithm>

void trim_gyro_data(std::vector<std::array<float, 3>> &data)
{
    // Remove leading and trailing low-activity samples
    const float ACTIVITY_THRESHOLD = 5.0f; // Adjust based on your gyro sensitivity

    // Find first significant movement
    auto start = std::find_if(data.begin(), data.end(), [](const array<float, 3> &sample)
                              { return abs(sample[0]) > ACTIVITY_THRESHOLD ||
                                       abs(sample[1]) > ACTIVITY_THRESHOLD ||
                                       abs(sample[2]) > ACTIVITY_THRESHOLD; });

    // Find last significant movement
    auto end = std::find_if(data.rbegin(), data.rend(), [](const array<float, 3> &sample)
                            { return abs(sample[0]) > ACTIVITY_THRESHOLD ||
                                     abs(sample[1]) > ACTIVITY_THRESHOLD ||
                                     abs(sample[2]) > ACTIVITY_THRESHOLD; })
                   .base();

    // Keep only the active portion of the gesture
    data = std::vector<std::array<float, 3>>(start, end);

    // Resample to standardize length if needed
    if (data.size() > 100)
    {
        std::vector<std::array<float, 3>> resampled;
        float step = static_cast<float>(data.size()) / 100;
        for (int i = 0; i < 100; i++)
        {
            int idx = static_cast<int>(i * step);
            resampled.push_back(data[idx]);
        }
        data = std::move(resampled);
    }
}

float normalize_signal(std::vector<float> &signal)
{
    float sum = std::accumulate(signal.begin(), signal.end(), 0.0f);
    float mean = sum / signal.size();

    // Remove mean (center the signal)
    for (float &value : signal)
    {
        value -= mean;
    }

    // Calculate standard deviation
    float sq_sum = std::inner_product(signal.begin(), signal.end(), signal.begin(), 0.0f);
    float stdev = std::sqrt(sq_sum / signal.size());

    // Normalize by standard deviation if it's not too close to zero
    if (stdev > 1e-6)
    {
        for (float &value : signal)
        {
            value /= stdev;
        }
    }

    return stdev;
}

float calculate_axis_correlation(const std::vector<float> &ref, const std::vector<float> &test)
{
    if (ref.empty() || test.empty())
        return 0.0f;

    // Create normalized copies of the signals
    std::vector<float> ref_norm = ref;
    std::vector<float> test_norm = test;

    normalize_signal(ref_norm);
    normalize_signal(test_norm);

    // Calculate correlation with a small window for time shift tolerance
    const int MAX_SHIFT = 5;
    float max_correlation = -1.0f;

    for (int shift = -MAX_SHIFT; shift <= MAX_SHIFT; ++shift)
    {
        float correlation = 0.0f;
        int count = 0;

        for (size_t i = 0; i < ref_norm.size(); ++i)
        {
            int j = i + shift;
            if (j >= 0 && j < static_cast<int>(test_norm.size()))
            {
                correlation += ref_norm[i] * test_norm[j];
                count++;
            }
        }

        if (count > 0)
        {
            correlation /= count;
            max_correlation = std::max(max_correlation, correlation);
        }
    }

    return max_correlation;
}

std::array<float, 3> calculateCorrelationVectors(
    const std::vector<std::array<float, 3>> &gesture_key,
    const std::vector<std::array<float, 3>> &unlocking_record)
{

    // Extract individual axis data
    std::vector<float> ref_x, ref_y, ref_z;
    std::vector<float> test_x, test_y, test_z;

    for (const auto &sample : gesture_key)
    {
        ref_x.push_back(sample[0]);
        ref_y.push_back(sample[1]);
        ref_z.push_back(sample[2]);
    }

    for (const auto &sample : unlocking_record)
    {
        test_x.push_back(sample[0]);
        test_y.push_back(sample[1]);
        test_z.push_back(sample[2]);
    }

    // Calculate correlation for each axis
    return {
        calculate_axis_correlation(ref_x, test_x),
        calculate_axis_correlation(ref_y, test_y),
        calculate_axis_correlation(ref_z, test_z)};
}