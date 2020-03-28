#ifndef TLC_LCD_KEYPAD_H
#define TLC_LCD_KEYPAD_H

extern char gLcdMsg[128];
extern char gLcdDetail[128];

bool LcdKeypad_Init();
void LcdKeypad_Process();

#endif // TLC_LCD_KEYPAD_H
