#include "dwin_lcd.h"

void DwinLCD::being(u32 baud)
{
    Serial.begin(baud);
}

void DwinLCD::ChangePage(u16 page)
{
    Serial.write(0x5A);
    Serial.write(0xA5);
    Serial.write(0x07);
    Serial.write(0x82);
    Serial.write((byte)0);
    Serial.write(0x84);
    Serial.write(0x5A);
    Serial.write(0x01);
    Serial.write(page >> 8);
    Serial.write(page & 0xFF);
}

void DwinLCD::SendData(u16 VP, u8 *buff, u8 size)
{
    Serial.write(0x5A);
    Serial.write(0xA5);
    Serial.write(size + 3);
    Serial.write(0x82);
    Serial.write(VP >> 8);
    Serial.write(VP & 0xFF);
    for (int i = 0; i < size; i++)
        Serial.write(buff[i]);
}

void DwinLCD::SendData(u16 VP, u16 data)
{
    Serial.write(0x5A);
    Serial.write(0xA5);
    Serial.write(0x04);
    Serial.write(0x82);
    Serial.write(VP >> 8);
    Serial.write(VP & 0xFF);
    Serial.write(data >> 8);
    Serial.write(data & 0xFF);
}

bool DwinLCD::ReadData(u16 VP, u8 *buff, u8 size, u32 timeout)
{

    Serial.write(0x5A);
    Serial.write(0xA5);
    Serial.write(0x04);
    Serial.write(0x83);
    Serial.write(VP >> 8);
    Serial.write(VP & 0xFF);
    Serial.write(size);

    u32 startTime = millis();

    while (Serial.available() < 7 + size)
    {
        if (millis() - startTime > timeout)
            return false;
        delay(1);
    }

    if (Serial.read() != 0x5A || Serial.read() != 0xA5 || Serial.read() != 0x08 || Serial.read() != 0x83 || Serial.read() != VP >> 8 || Serial.read() != VP & 0xFF || Serial.read() != size)
        return false;

    for (int i = 0; i < size; i++)
        buff[i] = Serial.read();

    return true;
}

short DwinLCD::ReadData(u8 *buff, u8 size, u32 timeout)
{
    u32 startTime = millis();

    while (!Serial.available())
    {
        if (millis() - startTime > timeout)
            return -1;
        delay(1);
    }

    if (Serial.read() != 0x5A || Serial.read() != 0xA5 || !Serial.read() || Serial.read() != 0xffffff83)
        return -1;

    short resultLen = 0;
    while (Serial.available() > 0)
        buff[resultLen++] = Serial.read();

    return resultLen;
}

bool DwinLCD::ReadPointer(u16 VP, u16 &data, u32 timeout)
{
    u8 buff[2];
    if (!ReadData(VP, buff, 2, timeout))
        return false;

    data = buff[0] << 8 | buff[1];
    return true;
}