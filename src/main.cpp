/**
 * @file main.cpp
 * @author Xhovani Mali (xxm202)
 * @brief Entry point to embedded systems final project on embedded sentry.
 * @version 0.1
 * @date 2024-12-15
 *
 *
 * @group Members:
 * - Xhovani Mali (xxm202)
 * - Shruti Pangare (stp8232)
 * - Temira Koenig (trk8600)
 * - Shruti Tulshidas Pangare
 */

#include <mbed.h>                     // Mbed core
#include <vector>                     // For vector usage
#include <array>                      // For array usage
#include "utilities.h"                // Utility functions
#include "gyro.h"                     // Gyroscope functions
#include "system_config.h"            // System configuration
#include "drivers/LCD_DISCO_F429ZI.h" // LCD driver
#include "drivers/TS_DISCO_F429ZI.h"  // Touch screen driver

InterruptIn gyroscope_interrupt(PA_2, PullDown);
InterruptIn user_command_button(USER_BUTTON, PullDown);

DigitalOut led_status_green(LED1);
DigitalOut led_status_red(LED2);

LCD_DISCO_F429ZI lcd; // LCD object
TS_DISCO_F429ZI ts; // Touch screen object

EventFlags flags; // Event flags

Timer timer; // Timer

float movingAverageFilter(float new_value, std::array<float, WINDOW_SIZE>& buffer, size_t& index, float& sum);
void normalize(vector<array<float, 3>>& data);

// Global variables for moving average filter
float gyro_buffer_x[WINDOW_SIZE] = {0};  // Buffers for x, y, z data
float gyro_buffer_y[WINDOW_SIZE] = {0};
float gyro_buffer_z[WINDOW_SIZE] = {0};
size_t gyro_index_x = 0, gyro_index_y = 0, gyro_index_z = 0;  // Index for each buffer
float gyro_sum_x = 0, gyro_sum_y = 0, gyro_sum_z = 0;  // Sum for each axis


/*******************************************************************************
 * Function Prototypes of LCD and Touch Screen
 * ****************************************************************************/
void draw_button(int x, int y, int width, int height, const char *label);
bool is_touch_inside_button(int touch_x, int touch_y, int button_x, int button_y, int button_width, int button_height);

void gyroscope_thread();
void touch_screen_thread();

bool storeGyroDataToFlash(vector<array<float, 3>> &gesture_key, uint32_t flash_address);
vector<array<float, 3>> readGyroDataFromFlash(uint32_t flash_address, size_t data_size);

// moving average filter will be defined after main()
float movingAverageFilter(float input, float display_buffer[], size_t N, size_t &index, float &sum);

/*******************************************************************************
 * ISR Callback Functions
 * ****************************************************************************/
void button_press() // Callback function for button press
{
    flags.set(ERASE_FLAG);
}
void onGyroDataReady() // Gyrscope data ready ISR
{
    flags.set(DATA_READY_FLAG);
}

/*******************************************************************************
 * @brief Global Variables
 * ****************************************************************************/
vector<array<float, 3>> gesture_key; // the gesture key
vector<array<float, 3>> unlocking_record; // the unlocking record

const int button1_x = 60;
const int button1_y = 80;
const int button1_width = 120;
const int button1_height = 50;
const char *button1_label = "RECORD";
const int button2_x = 60;
const int button2_y = 180;
const int button2_width = 120;
const int button2_height = 50;
const char *button2_label = "UNLOCK";
const int message_x = 5;
const int message_y = 30;
const char *message = "EMBEDDED SENTRY";
const int text_x = 5;
const int text_y = 270;
const char *text_0 = "NO KEY RECORDED";
const char *text_1 = "LOCKED";


int err = 0; // for error checking

/*******************************************************************************
 * @brief main function
 * ****************************************************************************/
int main()
{
    lcd.Clear(LCD_COLOR_BLACK);

    // Draw button 1
    draw_button(button1_x, button1_y, button1_width, button1_height, button1_label);

    // Draw button 2
    draw_button(button2_x, button2_y, button2_width, button2_height, button2_label);

    // Display the welcome message
    lcd.DisplayStringAt(message_x, message_y, (uint8_t *)message, CENTER_MODE);

    // initialize all interrupts
    user_command_button.rise(&button_press);
    gyroscope_interrupt.rise(&onGyroDataReady);

    if (gesture_key.empty())
    {
        led_status_red = 0;
        led_status_green = 1;  // Green LED indicates ready to record
        lcd.SetTextColor(LCD_COLOR_GREEN);
        lcd.DisplayStringAt(text_x, text_y, (uint8_t *)text_0, CENTER_MODE);
    }
    else
    {
        led_status_red = 1;    // Red LED indicates locked
        led_status_green = 0;
        lcd.SetTextColor(LCD_COLOR_RED);    // Use RED for locked status
        lcd.DisplayStringAt(text_x, text_y, (uint8_t *)text_1, CENTER_MODE);
    }

    // Create the gyroscope thread
    Thread key_saving;
    key_saving.start(callback(gyroscope_thread));

    // Create the touch screen thread
    Thread touch_thread;
    touch_thread.start(callback(touch_screen_thread));

    // keep main thread alive
    while (1)
    {
        ThisThread::sleep_for(100ms);
    }
}

/*******************************************************************************
 * @brief Gyroscope Gesture Key Saving Thread
 *
 * This thread handles the initialization, recording, saving, and unlocking of
 * gesture keys using the gyroscope. It also handles the LCD display updates and
 * LED status indicators during the process.
 *
 * ****************************************************************************/
void gyroscope_thread()
{
    // Initialize gyroscope configuration parameters
    Gyroscope_Init_Parameters init_parameters = {
            ODR_200_CUTOFF_50, // Output data rate
            INT2_DRDY,         // Interrupt configuration
            FULL_SCALE_500     // Full-scale selection
    };

    // Set up gyroscope's raw data
    Gyroscope_RawData raw_data;
    char display_buffer[50];

    // After gyroscope initialization
    printf("Gyroscope Initialized: ODR_200_CUTOFF_50, INT2_DRDY, FULL_SCALE_500\n");
    printf("Gyroscope Raw Data: x = %d, y = %d, z = %d\n", raw_data.x_raw, raw_data.y_raw, raw_data.z_raw);

    // Ensure the data-ready flag is set if the gyroscope interrupt is triggered
    if (!(flags.get() & DATA_READY_FLAG) && (gyroscope_interrupt.read() == 1))
    {
        flags.set(DATA_READY_FLAG);
        printf("DATA_READY_FLAG set.\n");
    }

    while (1)
    {
        vector<array<float, 3>> temp_key; // Temporary key to store recorded gyroscope data

        // Wait for a flag indicating recording, unlocking, or erasing actions
        auto flag_check = flags.wait_any(KEY_FLAG | UNLOCK_FLAG | ERASE_FLAG);
        printf("Waiting for flags: KEY_FLAG | UNLOCK_FLAG | ERASE_FLAG\n");
        printf("Flag check result: %ld\n", flag_check);

        // Handle key erasing action
        if (flag_check & ERASE_FLAG)
        {
            printf("Erasing gesture key...\n");
            sprintf(display_buffer, "Erasing....");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_YELLOW); // Yellow to indicate erasing
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            // Clear gesture key and unlocking record
            gesture_key.clear();
            unlocking_record.clear();

            // Display erasing completion message
            sprintf(display_buffer, "Key Erasing finish.");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_YELLOW); // Yellow color for completion
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            // Reset LED status and print message
            led_status_green = 1;
            led_status_red = 0;
            sprintf(display_buffer, "All Erasing finish.");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_YELLOW); // Yellow for success
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
        }

        // Handle key recording or unlocking actions
        if (flag_check & (KEY_FLAG | UNLOCK_FLAG))
        {
            printf("Preparing for recording...\n");
            sprintf(display_buffer, "Hold On");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_ORANGE); // Orange for "Hold On"
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            ThisThread::sleep_for(1s);

            // Calibrate gyroscope
            printf("Calibrating gyroscope...\n");
            sprintf(display_buffer, "Calibrating...");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_LIGHTGRAY); // Light gray to indicate calibration
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            // Initialize the gyroscope
            InitiateGyroscope(&init_parameters, &raw_data);
            printf("Gyroscope initialized. Raw data: x = %d, y = %d, z = %d\n", raw_data.x_raw, raw_data.y_raw, raw_data.z_raw);

            // Start recording gesture with countdown
            for (int i = 3; i > 0; --i)
            {
                sprintf(display_buffer, "Recording in %d...", i);
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_ORANGE); // Orange to indicate countdown
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
                ThisThread::sleep_for(1s);
            }

            sprintf(display_buffer, "Recording...");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_GREEN); // Green to indicate recording
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            // Gyro data recording loop (3 seconds)
            printf("Starting gyro data recording...\n");
            timer.start();
            while (timer.elapsed_time() < 3s)
            {
                flags.wait_all(DATA_READY_FLAG);
                GetCalibratedRawData();

                // Initialize smoothed values
                float smoothed_x = 0.0f;
                float smoothed_y = 0.0f;
                float smoothed_z = 0.0f;

                // Apply the moving average filter to smooth gyroscope data
                smoothed_x = movingAverageFilter(ConvertToDPS(raw_data.x_raw),
                                                 reinterpret_cast<array<float, 5> &>(gyro_buffer_x), gyro_index_x, gyro_sum_x);
                smoothed_y = movingAverageFilter(ConvertToDPS(raw_data.y_raw),
                                                 reinterpret_cast<array<float, 5> &>(gyro_buffer_y), gyro_index_y, gyro_sum_y);
                smoothed_z = movingAverageFilter(ConvertToDPS(raw_data.z_raw),
                                                 reinterpret_cast<array<float, 5> &>(gyro_buffer_z), gyro_index_z, gyro_sum_z);

                // Debug print to check raw and smoothed data
                printf("Raw Gyro Data: x = %d, y = %d, z = %d\n", raw_data.x_raw, raw_data.y_raw, raw_data.z_raw);
                printf("Smoothed Gyro Data: x = %f, y = %f, z = %f\n", smoothed_x, smoothed_y, smoothed_z);

                temp_key.push_back({ConvertToDPS(raw_data.x_raw), ConvertToDPS(raw_data.y_raw), ConvertToDPS(raw_data.z_raw)});
                ThisThread::sleep_for(50ms); // 20Hz sampling
            }
            timer.stop();
            timer.reset();

            // Debugging: Check collected data before trimming
            printf("Data Collected Before Trimming:\n");
            for (const auto& data_point : temp_key) {
                printf("x = %f, y = %f, z = %f\n", data_point[0], data_point[1], data_point[2]);
            }

            // Trim zero data
            trim_gyro_data(temp_key);

            // Debugging: Check data after trimming
            printf("Data After Trimming:\n");
            for (const auto& data_point : temp_key) {
                printf("x = %f, y = %f, z = %f\n", data_point[0], data_point[1], data_point[2]);
            }

            sprintf(display_buffer, "Finished...");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_GREEN); // Green for finished
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
        }

        // Handle saving or replacing gesture keys
        if (flag_check & KEY_FLAG)
        {
            printf("Saving gesture key...\n");
            if (gesture_key.empty())
            {
                sprintf(display_buffer, "Saving Key...");
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_LIGHTGREEN); // Light green for saving
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                // Save new gesture key
                gesture_key = temp_key;
                temp_key.clear();

                // Toggle LED to indicate saving
                led_status_red = 1;
                led_status_green = 0;

                sprintf(display_buffer, "Key saved...");
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_LIGHTGREEN); // Light green to confirm
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
            }
            else
            {
                printf("Replacing old gesture key...\n");
                sprintf(display_buffer, "Removing old key...");
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_ORANGE); // Orange to indicate replacement
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                ThisThread::sleep_for(1s);

                gesture_key.clear();
                gesture_key = temp_key;

                sprintf(display_buffer, "New key is saved.");
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_LIGHTGREEN); // Light green for success
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                temp_key.clear();

                led_status_red = 1;
                led_status_green = 0;
            }

            printf("Gesture Key Data:\n");
            for (auto &gesture : gesture_key) {
                printf("x = %f, y = %f, z = %f\n", gesture[0], gesture[1], gesture[2]);
            }

        }
        else if (flag_check & UNLOCK_FLAG)
        {
            printf("Unlocking gesture...\n");
            flags.clear(UNLOCK_FLAG);
            sprintf(display_buffer, "Unlocking...");
            lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
            lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
            lcd.SetTextColor(LCD_COLOR_LIGHTGRAY); // Light gray for unlocking
            lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

            unlocking_record = temp_key; // Save the unlocking record
            temp_key.clear(); // Clear temp_key

            if (gesture_key.empty())
            {
                sprintf(display_buffer, "NO KEY SAVED.");
                lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                lcd.SetTextColor(LCD_COLOR_RED); // Red for error
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                unlocking_record.clear();
                led_status_green = 1;
                led_status_red = 0;
            }
            else
            {
                int unlock_count = 0; // counter for the coordinates that are above threshold
                // Resize both vectors to the same size before calculation
                size_t target_size = std::min(gesture_key.size(), unlocking_record.size());
                gesture_key.resize(target_size);
                unlocking_record.resize(target_size);

                // Normalize both gesture and unlocking records
                normalize(gesture_key);
                normalize(unlocking_record);

                // Ensure that both vectors have the same size
                if (gesture_key.size() != unlocking_record.size()) {
                    printf("Error: Vectors have different sizes before correlation!\n");
                    size_t min_size = std::min(gesture_key.size(), unlocking_record.size());
                    gesture_key.resize(min_size);
                    unlocking_record.resize(min_size);
                    printf("Vectors resized to the same size: %zu\n", min_size);
                }

                array<float, 3> correlationResult = calculateCorrelationVectors(gesture_key, unlocking_record); // calculate correlation

                if (err != 0)
                {
                    printf("Error calculating correlation: vectors have different sizes\n");
                }
                else
                {
                    printf("Correlation values: x = %f, y = %f, z = %f\n", correlationResult[0], correlationResult[1], correlationResult[2]);
                    err = -1;

                    // Iterate through correlationResult to check if all values are above the threshold
                    for (size_t i = 0; i < correlationResult.size(); i++)
                    {
                        if (correlationResult[i] > CORRELATION_THRESHOLD) // Ensure the threshold is appropriate (e.g., 0.8 or higher)
                        {
                            unlock_count++;
                        }
                    }
                }

                // Update the display and LED status based on unlock result
                if (unlock_count == 1)  // All axes should match
                {
                    sprintf(display_buffer, "UNLOCK: SUCCESS");
                    lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                    lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                    lcd.SetTextColor(LCD_COLOR_GREEN); // Green for success
                    lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                    led_status_green = 1;
                    led_status_red = 0;
                }
                else
                {
                    sprintf(display_buffer, "UNLOCK: FAILED");
                    lcd.SetTextColor(LCD_COLOR_BLACK); // Set background color
                    lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear the line
                    lcd.SetTextColor(LCD_COLOR_RED); // Red for failure
                    lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);

                    led_status_green = 0;
                    led_status_red = 1;
                }

                // Clear unlocking record after the attempt
                unlocking_record.clear();

                printf("Unlocking Gesture Data:\n");
            }
        }

        ThisThread::sleep_for(50ms);
    }
}

/*******************************************************************************
 *
 * @brief touch screen thread
 *
 * ****************************************************************************/
void touch_screen_thread()
{
    // Add your touch screen initialization and handling code here
    TS_StateTypeDef ts_state;

    if (ts.Init(lcd.GetXSize(), lcd.GetYSize()) != TS_OK)
    {
        printf("Failed to initialize the touch screen!\r\n");
        return;
    }

    // initialize a string display_buffer that can be draw on the LCD to dispaly the status
    char display_buffer[50];

    while (1)
    {
        ts.GetState(&ts_state);
        if (ts_state.TouchDetected)
        {
            int touch_x = ts_state.X;
            int touch_y = ts_state.Y;

            // Check if the touch is inside record button
            if (is_touch_inside_button(touch_x, touch_y, button2_x, button2_y, button1_width, button1_height))
            {
                sprintf(display_buffer, "Recording Initiated...");
                lcd.SetTextColor(LCD_COLOR_BLACK);                  // Set the color to the background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear a specific line
                lcd.SetTextColor(LCD_COLOR_BLUE);                   // Reset the text color
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
                ThisThread::sleep_for(1s);
                flags.set(KEY_FLAG);
            }

            // Check if the touch is inside unlock button
            if (is_touch_inside_button(touch_x, touch_y, button1_x, button1_y, button2_width, button2_height))
            {
                sprintf(display_buffer, "Unlocking Initiated...");
                lcd.SetTextColor(LCD_COLOR_BLACK);                  // Set the color to the background color
                lcd.FillRect(0, text_y, lcd.GetXSize(), FONT_SIZE); // Clear a specific line
                lcd.SetTextColor(LCD_COLOR_BLUE);                   // Reset the text color
                lcd.DisplayStringAt(text_x, text_y, (uint8_t *)display_buffer, CENTER_MODE);
                ThisThread::sleep_for(1s);
                flags.set(UNLOCK_FLAG);
            }
        }
        ThisThread::sleep_for(10ms);
    }
}

/*******************************************************************************
 *
 * @brief draw button
 * @param x: x coordinate of the button
 * @param y: y coordinate of the button
 * @param width: width of the button
 * @param height: height of the button
 * @param label: label of the button
 *
 * ****************************************************************************/
void draw_button(int x, int y, int width, int height, const char *label)
{
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.FillRect(x, y, width, height);
    lcd.DisplayStringAt(x + width / 2 - strlen(label) * 19, y + height / 2 - 8, (uint8_t *)label, CENTER_MODE);
}

/*******************************************************************************
 *
 * @brief Check if the touch point is inside the button
 * @param touch_x: x coordinate of the touch point
 * @param touch_y: y coordinate of the touch point
 * @param button_x: x coordinate of the button
 * @param button_y: y coordinate of the button
 * @param button_width: width of the button
 * @param button_height: height of the button
 *
 * ****************************************************************************/
bool is_touch_inside_button(int touch_x, int touch_y, int button_x, int button_y, int button_width, int button_height)
{
    return (touch_x >= button_x && touch_x <= button_x + button_width &&
            touch_y >= button_y && touch_y <= button_y + button_height);
}

float movingAverageFilter(float new_value, std::array<float, WINDOW_SIZE>& buffer, size_t& index, float& sum) {
    // Subtract the old value from the sum and add the new value
    sum -= buffer[index];
    buffer[index] = new_value;
    sum += new_value;

    // Move the index in the buffer
    index = (index + 1) % WINDOW_SIZE;

    // Return the average
    return sum / WINDOW_SIZE;
}

// Function to normalize a vector
void normalize(vector<array<float, 3>>& data) {
    for (auto& point : data) {
        float magnitude = sqrt(point[0] * point[0] + point[1] * point[1] + point[2] * point[2]);
        if (magnitude > 0) {
            point[0] /= magnitude;
            point[1] /= magnitude;
            point[2] /= magnitude;
        }
    }
}
