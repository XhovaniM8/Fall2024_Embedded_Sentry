// gesture_handler.h
#ifndef GESTURE_HANDLER_H
#define GESTURE_HANDLER_H

#include <mbed.h>
#include <vector>
#include <array>
#include "gyroscope.h"
#include "lcd_touchscreen.h"

// Event flags definitions
#define KEY_FLAG 1
#define UNLOCK_FLAG 2
#define ERASE_FLAG 4
#define DATA_READY_FLAG 8
#define CORRELATION_THRESHOLD 0.3f

class GestureHandler
{
public:
    GestureHandler(EventFlags &flags, DigitalOut &green_led, DigitalOut &red_led);
    void initInterrupts(InterruptIn &user_button, InterruptIn &gyro_int2);
    void gyroscopeThread();
    void touchScreenThread();

private:
    EventFlags &flags;
    DigitalOut &green_led;
    DigitalOut &red_led;
    Timer timer;
    vector<array<float, 3>> gesture_key;
    vector<array<float, 3>> unlocking_record;

    void handleErase();
    void handleRecording(vector<array<float, 3>> &temp_key);
    void handleKeyRecording(vector<array<float, 3>> &temp_key);
    void handleUnlocking(vector<array<float, 3>> &temp_key);
    static void buttonPressISR(GestureHandler *handler);
    static void gyroDataReadyISR(GestureHandler *handler);
};

#endif // GESTURE_HANDLER_H