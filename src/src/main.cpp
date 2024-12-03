#include "lib/drivers/LCD_DISCO_F429ZI.h"
#include "lib/drivers/TS_DISCO_F429ZI.h"
#include "include/system_config.h"
#include "include/gesture_handler.h"
#include "include/lcd_touchscreen.h"
#include "include/flash_storage.h"
#include "include/utilities.h"

// Hardware initialization
InterruptIn gyro_int2(PA_2, PullDown);
InterruptIn user_button(USER_BUTTON, PullDown);
DigitalOut green_led(LED1);
DigitalOut red_led(LED2);
LCD_DISCO_F429ZI display;

// Global flags for event handling
volatile uint32_t flags = 0;

// Global gesture handler instance
GestureHandler *g_gestureHandler = nullptr;

int main()
{
    // Initialize display
    display.Clear(LCD_COLOR_BLACK);

    // Create gesture handler
    GestureHandler gestureHandler(flags, green_led, red_led);
    g_gestureHandler = &gestureHandler;

    // Initialize interrupts
    gestureHandler.initInterrupts(user_button, gyro_int2);

    // Set initial display state
    if (gestureHandler.isLocked())
    {
        red_led = 1;
        green_led = 0;
        display.DisplayStringAt(0, 0, (uint8_t *)"Locked", CENTER_MODE);
    }
    else
    {
        red_led = 0;
        green_led = 1;
        display.DisplayStringAt(0, 0, (uint8_t *)"No Key Recorded", CENTER_MODE);
    }

    // Timer for periodic execution
    Timer timer;
    timer.start();

    const int GYRO_UPDATE_INTERVAL_MS = 50;   // Interval for gyroscope updates
    const int TOUCH_UPDATE_INTERVAL_MS = 100; // Interval for touchscreen updates
    uint32_t last_gyro_update = 0;
    uint32_t last_touch_update = 0;

    // Main loop
    while (1)
    {
        uint32_t elapsed_time = chrono::duration_cast<chrono::milliseconds>(timer.elapsed_time()).count();

        // Update gyroscope at regular intervals
        if (elapsed_time - last_gyro_update >= GYRO_UPDATE_INTERVAL_MS)
        {
            gestureHandler.gyroscopeThreadFn();
            last_gyro_update = elapsed_time;
        }

        // Update touchscreen at regular intervals
        if (elapsed_time - last_touch_update >= TOUCH_UPDATE_INTERVAL_MS)
        {
            gestureHandler.touchScreenThreadFn();
            last_touch_update = elapsed_time;
        }

        // Small sleep to reduce CPU usage
        wait_us(1000); // Sleep for 1ms
    }
}
