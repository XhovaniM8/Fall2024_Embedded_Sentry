// lcd_touchscreen.cpp
#include "lcd_touchscreen.h"

LCDTouchscreen display;

LCDTouchscreen::LCDTouchscreen()
{
}

void LCDTouchscreen::init()
{
    lcd.Clear(LCD_COLOR_BLACK);
    if (ts.Init(lcd.GetXSize(), lcd.GetYSize()) != TS_OK)
    {
        printf("Failed to initialize touch screen!\r\n");
    }
    drawButtons();
    displayMessage(WELCOME_MESSAGE, MESSAGE_X, MESSAGE_Y);
}

void LCDTouchscreen::drawButtons()
{
    drawButton(BUTTON1_X, BUTTON1_Y, BUTTON1_WIDTH, BUTTON1_HEIGHT, "RECORD");
    drawButton(BUTTON2_X, BUTTON2_Y, BUTTON2_WIDTH, BUTTON2_HEIGHT, "UNLOCK");
}

void LCDTouchscreen::drawButton(int x, int y, int width, int height, const char *label)
{
    lcd.SetTextColor(LCD_COLOR_BLUE);
    lcd.FillRect(x, y, width, height);
    lcd.DisplayStringAt(x + width / 2 - strlen(label) * 19,
                        y + height / 2 - 8,
                        (uint8_t *)label,
                        CENTER_MODE);
}

void LCDTouchscreen::displayMessage(const char *message, int x, int y)
{
    lcd.DisplayStringAt(x, y, (uint8_t *)message, CENTER_MODE);
}

void LCDTouchscreen::updateDisplayMessage(const char *message, uint16_t color)
{
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(0, TEXT_Y, lcd.GetXSize(), FONT_SIZE);
    lcd.SetTextColor(color);
    lcd.DisplayStringAt(TEXT_X, TEXT_Y, (uint8_t *)message, CENTER_MODE);
}

void LCDTouchscreen::clearLine(int y)
{
    lcd.SetTextColor(LCD_COLOR_BLACK);
    lcd.FillRect(0, y, lcd.GetXSize(), FONT_SIZE);
    lcd.SetTextColor(LCD_COLOR_BLUE);
}

bool LCDTouchscreen::checkTouch(uint8_t &flag)
{
    TS_StateTypeDef ts_state;
    ts.GetState(&ts_state);

    if (ts_state.TouchDetected)
    {
        int touch_x = ts_state.X;
        int touch_y = ts_state.Y;

        if (isTouchInsideButton(touch_x, touch_y, BUTTON2_X, BUTTON2_Y,
                                BUTTON1_WIDTH, BUTTON1_HEIGHT))
        {
            updateDisplayMessage("Recording Initiated...");
            flag = 1; // KEY_FLAG
            return true;
        }

        if (isTouchInsideButton(touch_x, touch_y, BUTTON1_X, BUTTON1_Y,
                                BUTTON2_WIDTH, BUTTON2_HEIGHT))
        {
            updateDisplayMessage("Unlocking Initiated...");
            flag = 2; // UNLOCK_FLAG
            return true;
        }
    }
    return false;
}

bool LCDTouchscreen::isTouchInsideButton(int touch_x, int touch_y, int button_x,
                                         int button_y, int button_width, int button_height)
{
    return (touch_x >= button_x && touch_x <= button_x + button_width &&
            touch_y >= button_y && touch_y <= button_y + button_height);
}