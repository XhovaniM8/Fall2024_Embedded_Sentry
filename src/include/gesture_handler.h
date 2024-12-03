#ifndef GESTURE_HANDLER_H
#define GESTURE_HANDLER_H

#include "system_config.h" // Contains definitions for flags
#include "gyroscope.h"
#include <mbed.h>
#include <vector>
#include <array>

// Make the handler instance globally accessible
extern GestureHandler *g_gestureHandler;

class GestureHandler
{
private:
    volatile uint32_t &flags;
    DigitalOut &green_led;
    DigitalOut &red_led;
    Timer timer;

    // Gesture data storage
    std::vector<std::array<float, 3>> gesture_key;
    std::vector<std::array<float, 3>> unlocking_record;

    // Gyroscope parameters
    Gyroscope_Init_Parameters init_parameters;
    Gyroscope_RawData raw_data;

    void handleErase();
    void handleRecording(std::vector<std::array<float, 3>> &temp_key);
    void handleKeyRecording(std::vector<std::array<float, 3>> &temp_key);
    void handleUnlocking(std::vector<std::array<float, 3>> &temp_key);

    static void buttonPressISR(GestureHandler *handler);
    static void gyroDataReadyISR(GestureHandler *handler);

public:
    GestureHandler(volatile uint32_t &flags, DigitalOut &green_led, DigitalOut &red_led);
    void initInterrupts(InterruptIn &user_button, InterruptIn &gyro_int2);
    void gyroscopeThreadFn();
    void touchScreenThreadFn();
    bool isLocked() const { return !gesture_key.empty(); }

    // Static thread functions that can be used with simple callback
    static void gyroscope_thread()
    {
        if (g_gestureHandler)
        {
            g_gestureHandler->gyroscopeThreadFn();
        }
    }

    static void touch_screen_thread()
    {
        if (g_gestureHandler)
        {
            g_gestureHandler->touchScreenThreadFn();
        }
    }
};

#endif // GESTURE_HANDLER_H
