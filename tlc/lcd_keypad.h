///
/// \file       lcd_keypad.h
/// \brief      The Lung Carburetor Firmware lcd and keypad module
///
/// \author     Sylvain Brisebois
/// \defgroup   lcdkeypad LCD Display
#ifndef TLC_LCD_KEYPAD_H
#define TLC_LCD_KEYPAD_H

#ifdef ENABLE_LCD

extern char gLcdMsg[128];
extern char gLcdDetail[128];

#endif

/// \fn bool LcdKeypad_Init()
/// \brief Initialize lcd and keypad
bool LcdKeypad_Init();

/// \fn bool LcdKeypad_Process()
/// \brief Process lcd display and keypad processing
void LcdKeypad_Process();

#endif // TLC_LCD_KEYPAD_H
