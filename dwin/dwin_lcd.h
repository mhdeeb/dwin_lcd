#ifndef DWIN_LCD_H
#define DWIN_LCD_H

#include "Arduino.h"

class DwinLCD
{
public:
    void being(u32 baud);

    void ChangePage(u16 page);

    void SendData(u16 VP, u8 *buff, u8 size);

    void SendData(u16 VP, u16 data);

    bool ReadData(u16 VP, u8 *buff, u8 size, u32 timeout = 1000);

    bool ReadPointer(u16 VP, u16 &data, u32 timeout = 1000);

    short ReadData(u8 *buff, u8 size, u32 timeout = 1000);

    short ReadData2(u8 *buff, u8 size, u32 timeout = 1000);
};

#endif // DWIN_LCD_H
