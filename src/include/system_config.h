#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <mbed.h>


// Event flags for system state
#define FLAG_KEY_RECORD (1U << 0)
#define FLAG_UNLOCK (1U << 1)
#define FLAG_ERASE (1U << 2)
#define FLAG_DATA_READY (1U << 3)

// System constants
#define LCD_FONT_SIZE 16
#define GESTURE_MAX_TIME 5000 // Maximum gesture recording time in ms
#define GESTURE_MIN_TIME 500  // Minimum gesture recording time in ms
#define SAMPLE_RATE_MS 50     // 20Hz sampling rate
#define CORRELATION_MIN 0.3f  // Minimum correlation threshold for match

// Hardware pin definitions
#define USER_BUTTON PA_0
#define GYRO_INT_PIN PA_2
#define GYRO_CS_PIN PC_1
#define GYRO_MOSI_PIN PF_9
#define GYRO_MISO_PIN PF_8
#define GYRO_SCK_PIN PF_7

// Error codes
enum SystemError
{
    ERROR_NONE = 0,
    ERROR_GYRO_INIT = -1,
    ERROR_GESTURE_SHORT = -2,
    ERROR_GESTURE_INVALID = -3,
    ERROR_CALIBRATION = -4
};

#endif 