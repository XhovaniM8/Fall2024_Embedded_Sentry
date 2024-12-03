#include "flash_storage.h"
#include <mbed.h>

bool storeGyroDataToFlash(const vector<array<float, 3>> &gesture_key, uint32_t flash_address)
{
    FlashIAP flash;
    flash.init();

    uint32_t data_size = gesture_key.size() * sizeof(array<float, 3>);
    flash.erase(flash_address, data_size);

    int write_result = flash.program(gesture_key.data(), flash_address, data_size);

    flash.deinit();
    return write_result == 0;
}

vector<array<float, 3>> readGyroDataFromFlash(uint32_t flash_address, size_t data_size)
{
    vector<array<float, 3>> gesture_key(data_size);

    FlashIAP flash;
    flash.init();
    flash.read(gesture_key.data(), flash_address, data_size * sizeof(array<float, 3>));
    flash.deinit();

    return gesture_key;
}
