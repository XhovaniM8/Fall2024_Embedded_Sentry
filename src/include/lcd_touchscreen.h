// lcd_touchscreen.h
#ifndef LCD_TOUCHSCREEN_H
#define LCD_TOUCHSCREEN_H

#include <mbed.h>
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"

// Display Constants
#define FONT_SIZE 16

// Button Constants
#define BUTTON1_X 60
#define BUTTON1_Y 80
#define BUTTON1_WIDTH 120
#define BUTTON1_HEIGHT 50
#define BUTTON2_X 60
#define BUTTON2_Y 180
#define BUTTON2_WIDTH 120
#define BUTTON2_HEIGHT 50

// Text Position Constants
#define TEXT_X 5
#define TEXT_Y 270
#define MESSAGE_X 5
#define MESSAGE_Y 30

// Message Constants
#define WELCOME_MESSAGE "GESTURE UNLOCKER"
#define NO_KEY_MESSAGE "NO KEY RECORDED"
#define LOCKED_MESSAGE "LOCKED"

class LCDTouchscreen
{
public:
    LCDTouchscreen();
    void init();
    void drawButtons();
    void displayMessage(const char *message, int x, int y);
    void updateDisplayMessage(const char *message, uint16_t color = LCD_COLOR_BLUE);
    void clearLine(int y);
    bool checkTouch(uint8_t &flag);
    bool isTouchInsideButton(int touch_x, int touch_y, int button_x, int button_y,
                             int button_width, int button_height);

private:
    LCD_DISCO_F429ZI lcd;
    TS_DISCO_F429ZI ts;
    void drawButton(int x, int y, int width, int height, const char *label);
};

extern LCDTouchscreen display;

#endif // LCD_TOUCHSCREEN_H