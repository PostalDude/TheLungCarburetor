///
/// \file       lcd_keypad.cpp
/// \brief      The Lung Carburetor Firmware lcd and keypad module
///
/// \author     Sylvain Brisebois
/// \ingroup    lcdkeypad
#include "common.h"
#include "lcd_keypad.h"

#include <Adafruit_RGBLCDShield.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

char gLcdMsg[128];
char gLcdDetail[128];

bool LcdKeypad_Init()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    lcd.setBacklight(WHITE);
    gLcdMsg[0] = '\0';
    gLcdDetail[0] = '\0';

    return true;
}

void LcdKeypad_Process()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 0);
    lcd.print(gLcdMsg);

    // print the number of seconds since reset:
    lcd.setCursor(5, 1);
    lcd.print(gLcdDetail);

    uint8_t buttons = lcd.readButtons();
    if (buttons)
    {
        lcd.clear();
        lcd.setCursor(0,0);
        if (buttons & BUTTON_UP)
        {
          lcd.print("UP ");
          lcd.setBacklight(RED);
        }
        if (buttons & BUTTON_DOWN)
        {
          lcd.print("DOWN ");
          lcd.setBacklight(YELLOW);
        }
        if (buttons & BUTTON_LEFT)
        {
          lcd.print("LEFT ");
          lcd.setBacklight(GREEN);
        }
        if (buttons & BUTTON_RIGHT)
        {
          lcd.print("RIGHT ");
          lcd.setBacklight(TEAL);
        }
        if (buttons & BUTTON_SELECT)
        {
          lcd.print("SELECT ");
          lcd.setBacklight(VIOLET);
        }
    }
}
