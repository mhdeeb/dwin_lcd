#include "dwin_lcd.h"

DwinLCD::DwinLCD(u32 baud, u8 RX, u8 TX) : softSerial(RX, TX)
{
    softSerial.begin(baud);
}

void DwinLCD::ChangePage(u16 page)
{
    softSerial.write(0x5A);
    softSerial.write(0xA5);
    softSerial.write(0x07);
    softSerial.write(0x82);
    softSerial.write((byte)0);
    softSerial.write(0x84);
    softSerial.write(0x5A);
    softSerial.write(0x01);
    softSerial.write(page >> 8);
    softSerial.write(page & 0xFF);
}

void DwinLCD::SendData(u16 VP, u8 *buff, u8 size)
{
    softSerial.write(0x5A);
    softSerial.write(0xA5);
    softSerial.write(size + 3);
    softSerial.write(0x82);
    softSerial.write(VP >> 8);
    softSerial.write(VP & 0xFF);
    for (int i = 0; i < size; i++)
        softSerial.write(buff[i]);
}

void DwinLCD::SendData(u16 VP, u8 data)
{
    softSerial.write(0x5A);
    softSerial.write(0xA5);
    softSerial.write(0x04);
    softSerial.write(0x82);
    softSerial.write(VP >> 8);
    softSerial.write(VP & 0xFF);
    softSerial.write(data);
}

bool DwinLCD::ReadData(u16 VP, u8 *buff, u8 size, u32 timeout)
{

    softSerial.write(0x5A);
    softSerial.write(0xA5);
    softSerial.write(0x04);
    softSerial.write(0x83);
    softSerial.write(VP >> 8);
    softSerial.write(VP & 0xFF);
    softSerial.write(size);

    u32 startTime = millis();

    while (softSerial.available() < 7 + size)
    {
        if (millis() - startTime > timeout)
            return false;
        delay(1);
    }

    if (softSerial.read() != 0x5A || softSerial.read() != 0xA5 || softSerial.read() != 0x08 || softSerial.read() != 0x83 || softSerial.read() != VP >> 8 || softSerial.read() != VP & 0xFF || softSerial.read() != size)
        return false;

    for (int i = 0; i < size; i++)
        buff[i] = softSerial.read();

    return true;
}

short DwinLCD::ReadData(u8 *buff, u8 size, u32 timeout)
{
    u32 startTime = millis();

    while (!softSerial.available())
    {
        if (millis() - startTime > timeout)
            return -1;
        delay(1);
    }

    if (softSerial.read() != 0x5A || softSerial.read() != 0xA5)
        return -1;

    u8 size = softSerial.read();

    if (softSerial.read() != 0xffffff83)
        return -1;

    short resultLen = 0;
    while (softSerial.available() > 0)
        buff[resultLen++] = softSerial.read();

    return resultLen;
}