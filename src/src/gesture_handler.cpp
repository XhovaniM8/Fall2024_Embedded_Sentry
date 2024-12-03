#include "include/gesture_handler.h"

GestureHandler::GestureHandler(EventFlags &flags, DigitalOut &green_led, DigitalOut &red_led)
    : flags(flags), green_led(green_led), red_led(red_led)
{
    // Initialize gyroscope parameters
    init_parameters.conf1 = ODR_200_CUTOFF_50;
    init_parameters.conf3 = INT2_DRDY;
    init_parameters.conf4 = FULL_SCALE_500;
}

void GestureHandler::initInterrupts(InterruptIn &user_button, InterruptIn &gyro_int2)
{
    user_button.rise(callback(buttonPressISR, this));
    gyro_int2.rise(callback(gyroDataReadyISR, this));

    if (!(flags.get() & DATA_READY_FLAG) && (gyro_int2.read() == 1))
    {
        flags.set(DATA_READY_FLAG);
    }
}

void GestureHandler::buttonPressISR(GestureHandler *handler)
{
    handler->flags.set(ERASE_FLAG);
}

void GestureHandler::gyroDataReadyISR(GestureHandler *handler)
{
    handler->flags.set(DATA_READY_FLAG);
}

void GestureHandler::handleErase()
{
    display.updateDisplayMessage("Erasing....");
    gesture_key.clear();
    unlocking_record.clear();
    green_led = 1;
    red_led = 0;
    display.updateDisplayMessage("All Erasing finish.");
}

void GestureHandler::handleRecording(vector<array<float, 3>> &temp_key)
{
    display.updateDisplayMessage("Hold On");
    ThisThread::sleep_for(1s);

    display.updateDisplayMessage("Calibrating...");
    InitiateGyroscope(&init_parameters, &raw_data);

    const char *countdowns[] = {"Recording in 3...", "Recording in 2...", "Recording in 1..."};
    for (const char *msg : countdowns)
    {
        display.updateDisplayMessage(msg);
        ThisThread::sleep_for(1s);
    }

    display.updateDisplayMessage("Recording...");

    timer.start();
    while (timer.elapsed_time() < 5s)
    {
        flags.wait_all(DATA_READY_FLAG);
        GetCalibratedRawData();
        temp_key.push_back({ConvertToDPS(raw_data.x_raw),
                            ConvertToDPS(raw_data.y_raw),
                            ConvertToDPS(raw_data.z_raw)});
        ThisThread::sleep_for(50ms);
    }
    timer.stop();
    timer.reset();

    trim_gyro_data(temp_key);
    display.updateDisplayMessage("Finished...");
}

void GestureHandler::trim_gyro_data(vector<array<float, 3>> &data)
{
    // Remove noise and normalize data
    // Implementation details here
}

float GestureHandler::calculateCorrelationVectors(
    const vector<array<float, 3>> &ref,
    const vector<array<float, 3>> &test)
{
    // Implement correlation calculation
    // Return correlation coefficient
    return 0.0f; // Placeholder
}

void GestureHandler::gyroscopeThread()
{
    while (1)
    {
        vector<array<float, 3>> temp_key;
        auto flag_check = flags.wait_any(KEY_FLAG | UNLOCK_FLAG | ERASE_FLAG);

        if (flag_check & ERASE_FLAG)
        {
            handleErase();
        }

        if (flag_check & (KEY_FLAG | UNLOCK_FLAG))
        {
            handleRecording(temp_key);
        }

        if (flag_check & KEY_FLAG)
        {
            handleKeyRecording(temp_key);
        }
        else if (flag_check & UNLOCK_FLAG)
        {
            handleUnlocking(temp_key);
        }

        ThisThread::sleep_for(100ms);
    }
}