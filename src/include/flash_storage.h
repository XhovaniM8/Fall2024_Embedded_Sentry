#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <vector>
#include <array>

bool storeGyroDataToFlash(const std::vector<std::array<float, 3>> &gesture_key, uint32_t flash_address);
std::vector<std::array<float, 3>> readGyroDataFromFlash(uint32_t flash_address, size_t data_size);

#endif // FLASH_STORAGE_H
