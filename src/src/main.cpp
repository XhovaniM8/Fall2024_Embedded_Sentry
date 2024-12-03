// main.cpp
#include <mbed.h>
#include "gyroscope.h"
#include "lcd_touchscreen.h"

// Event flags
#define KEY_FLAG 1
#define UNLOCK_FLAG 2
#define ERASE_FLAG 4
#define DATA_READY_FLAG 8

#define CORRELATION_THRESHOLD 0.3f

InterruptIn gyro_int2(PA_2, PullDown);
InterruptIn user_button(USER_BUTTON, PullDown);
DigitalOut green_led(LED1);
DigitalOut red_led(LED2);
EventFlags flags;
Timer timer;

// Global Variables
vector<array<float, 3>> gesture_key;
vector<array<float, 3>> unlocking_record;
int err = 0;

// Forward declarations
void gyroscope_thread();
void touch_screen_thread();

// ISR Callbacks
void button_press()
{
  flags.set(ERASE_FLAG);
}

void onGyroDataReady()
{
  flags.set(DATA_READY_FLAG);
}

int main()
{
  // Initialize LCD and touch screen
  display.init();

  // Initialize interrupts
  user_button.rise(&button_press);
  gyro_int2.rise(&onGyroDataReady);

  // Initialize LED states based on gesture key
  if (gesture_key.empty())
  {
    red_led = 0;
    green_led = 1;
    display.displayMessage(NO_KEY_MESSAGE, TEXT_X, TEXT_Y);
  }
  else
  {
    red_led = 1;
    green_led = 0;
    display.displayMessage(LOCKED_MESSAGE, TEXT_X, TEXT_Y);
  }

  // Create threads
  Thread key_saving;
  Thread touch_thread;

  key_saving.start(callback(gyroscope_thread));
  touch_thread.start(callback(touch_screen_thread));

  // Keep main thread alive
  while (1)
  {
    ThisThread::sleep_for(100ms);
  }
}

void gyroscope_thread()
{
  Gyroscope_Init_Parameters init_parameters;
  init_parameters.conf1 = ODR_200_CUTOFF_50;
  init_parameters.conf3 = INT2_DRDY;
  init_parameters.conf4 = FULL_SCALE_500;

  Gyroscope_RawData raw_data;

  // Handle initial gyro state
  if (!(flags.get() & DATA_READY_FLAG) && (gyro_int2.read() == 1))
  {
    flags.set(DATA_READY_FLAG);
  }

  while (1)
  {
    vector<array<float, 3>> temp_key;
    auto flag_check = flags.wait_any(KEY_FLAG | UNLOCK_FLAG | ERASE_FLAG);

    if (flag_check & ERASE_FLAG)
    {
      display.updateDisplayMessage("Erasing....");
      gesture_key.clear();
      display.updateDisplayMessage("Key Erasing finish.");
      unlocking_record.clear();
      green_led = 1;
      red_led = 0;
      display.updateDisplayMessage("All Erasing finish.");
    }

    if (flag_check & (KEY_FLAG | UNLOCK_FLAG))
    {
      display.updateDisplayMessage("Hold On");
      ThisThread::sleep_for(1s);
      display.updateDisplayMessage("Calibrating...");

      InitiateGyroscope(&init_parameters, &raw_data);

      // Recording countdown
      const char *countdowns[] = {"Recording in 3...", "Recording in 2...", "Recording in 1..."};
      for (const char *msg : countdowns)
      {
        display.updateDisplayMessage(msg);
        ThisThread::sleep_for(1s);
      }

      display.updateDisplayMessage("Recording...");

      // Record gyro data
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

    // Handle key recording
    if (flag_check & KEY_FLAG)
    {
      handleKeyRecording(temp_key);
    }
    // Handle unlock attempt
    else if (flag_check & UNLOCK_FLAG)
    {
      handleUnlocking(temp_key);
    }

    ThisThread::sleep_for(100ms);
  }
}

void touch_screen_thread()
{
  uint8_t flag;
  while (1)
  {
    if (display.checkTouch(flag))
    {
      flags.set(flag);
    }
    ThisThread::sleep_for(10ms);
  }
}

// Helper functions
void handleKeyRecording(vector<array<float, 3>> &temp_key)
{
  if (gesture_key.empty())
  {
    display.updateDisplayMessage("Saving Key...");
    gesture_key = temp_key;
    temp_key.clear();
    red_led = 1;
    green_led = 0;
    display.updateDisplayMessage("Key saved...");
  }
  else
  {
    display.updateDisplayMessage("Removing old key...");
    ThisThread::sleep_for(1s);
    gesture_key = temp_key;
    temp_key.clear();
    red_led = 1;
    green_led = 0;
    display.updateDisplayMessage("New key is saved.");
  }
}

void handleUnlocking(vector<array<float, 3>> &temp_key)
{
  flags.clear(UNLOCK_FLAG);
  display.updateDisplayMessage("Unlocking...");

  unlocking_record = temp_key;
  temp_key.clear();

  if (gesture_key.empty())
  {
    display.updateDisplayMessage("NO KEY SAVED.");
    unlocking_record.clear();
    green_led = 1;
    red_led = 0;
    return;
  }

  array<float, 3> correlationResult = calculateCorrelationVectors(gesture_key, unlocking_record);
  int unlock = 0;

  for (float value : correlationResult)
  {
    if (value > CORRELATION_THRESHOLD)
      unlock++;
  }

  if (unlock == 3)
  {
    display.updateDisplayMessage("UNLOCK: SUCCESS");
    green_led = 1;
    red_led = 0;
  }
  else
  {
    display.updateDisplayMessage("UNLOCK: FAILED");
    green_led = 0;
    red_led = 1;
  }

  unlocking_record.clear();
}