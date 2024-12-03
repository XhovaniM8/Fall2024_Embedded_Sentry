// gesture_handler.cpp
#include "gesture_handler.h"

GestureHandler::GestureHandler(EventFlags& flags, DigitalOut& green_led, DigitalOut& red_led)
    : flags(flags), green_led(green_led), red_led(red_led) {
}

void GestureHandler::initInterrupts(InterruptIn& user_button, InterruptIn& gyro_int2) {
    user_button.rise(callback(buttonPressISR, this));
    gyro_int2.rise(callback(gyroDataReadyISR, this));
    
    if (!(flags.get() & DATA_READY_FLAG) && (gyro_int2.read() == 1)) {
        flags.set(DATA_READY_FLAG);
    }
}

void GestureHandler::buttonPressISR(GestureHandler* handler) {
    handler->flags.set(ERASE_FLAG);
}

void GestureHandler::gyroDataReadyISR(GestureHandler* handler) {
    handler->flags.set(DATA_READY_FLAG);
}

void GestureHandler::gyroscopeThread() {
    Gyroscope_Init_Parameters init_parameters;
    init_parameters.conf1 = ODR_200_CUTOFF_50;
    init_parameters.conf3 = INT2_DRDY;
    init_parameters.conf4 = FULL_SCALE_500;
    
    Gyroscope_RawData raw_data;
    
    while (1) {
        vector<array<float, 3>> temp_key;
        auto flag_check = flags.wait_any(KEY_FLAG | UNLOCK_FLAG | ERASE_FLAG);
        
        if (flag_check & ERASE_FLAG) {
            handleErase();
        }
        
        if (flag_check & (KEY_FLAG | UNLOCK_FLAG)) {
            handleRecording(temp_key);
        }
        
        if (flag_check & KEY_FLAG) {
            handleKeyRecording(temp_key);
        } else if (flag_check & UNLOCK_FLAG) {
            handleUnlocking(temp_key);
        }
        
        ThisThread::sleep_for(100ms);
    }
}

void GestureHandler::handleErase() {
    display.updateDisplayMessage("Erasing....");
    gesture_key.clear();
    unlocking_record.clear();
    green_led = 1;
    red_led = 0;
    display.updateDisplayMessage("All Erasing finish.");
}

void GestureHandler::handleRecording(vector<array<float, 3>>& temp_key) {
    display.updateDisplayMessage("Hold On");
    ThisThread::sleep_for(1s);
    
    display.updateDisplayMessage("Calibrating...");
    InitiateGyroscope(&init_parameters, &raw_data);
    
    // Recording countdown
    const char* countdowns[] = {"Recording in 3...", "Recording in 2...", "Recording in 1..."};
    for (const char* msg : countdowns) {
        display.updateDisplayMessage(msg);
        ThisThread::sleep_for(1s);
    }
    
    display.updateDisplayMessage("Recording...");
    
    // Record data
    timer.start();
    while (timer.elapsed_time() < 5s) {
        flags.wait_all(DATA_READY_FLAG);
        GetCalibratedRawData();
        temp_key.push_back({
            ConvertToDPS(raw_data.x_raw),
            ConvertToDPS(raw_data.y_raw),
            ConvertToDPS(raw_data.z_raw)
        });
        ThisThread::sleep_for(50ms);
    }
    timer.stop();
    timer.reset();
    
    trim_gyro_data(temp_key);
    display.updateDisplayMessage("Finished...");
}

void GestureHandler::handleKeyRecording(vector<array<float, 3>>& temp_key) {
    if (gesture_key.empty()) {
        display.updateDisplayMessage("Saving Key...");
        gesture_key = temp_key;
        red_led = 1;
        green_led = 0;
        display.updateDisplayMessage("Key saved...");
    } else {
        display.updateDisplayMessage("Removing old key...");
        ThisThread::sleep_for(1s);
        gesture_key = temp_key;
        red_led = 1;
        green_led = 0;
        display.updateDisplayMessage("New key is saved.");
    }
    temp_key.clear();
}

void GestureHandler::handleUnlocking(vector<array<float, 3>>& temp_key) {
    flags.clear(UNLOCK_FLAG);
    display.updateDisplayMessage("Unlocking...");
    
    unlocking_record = temp_key;
    temp_key.clear();
    
    if (gesture_key.empty()) {
        display.updateDisplayMessage("NO KEY SAVED.");
        unlocking_record.clear();
        green_led = 1;
        red_led = 0;
        return;
    }
    
    array<float, 3> correlationResult = calculateCorrelationVectors(gesture_key, unlocking_record);
    int unlock = 0;
    
    for (float value : correlationResult) {
        if (value > CORRELATION_THRESHOLD) unlock++;
    }
    
    if (unlock == 3) {
        display.updateDisplayMessage("UNLOCK: SUCCESS");
        green_led = 1;
        red_led = 0;
    } else {
        display.updateDisplayMessage("UNLOCK: FAILED");
        green_led = 0;
        red_led = 1;
    }
    unlocking_record.clear();
}

void GestureHandler::touchScreenThread() {
    uint8_t flag;
    while (1) {
        if (display.checkTouch(flag)) {
            flags.set(flag);
        }
        ThisThread::sleep_for(10ms);
    }
}