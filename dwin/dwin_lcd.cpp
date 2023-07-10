#include "dwin_lcd.h"

void DwinLCD::being(u32 baud)
{
    Serial.begin(baud);
}

void DwinLCD::ChangePage(u16 page)
{
    u8 buff[] = {
        0x5A,
        0xA5,
        0x07,
        0x82,
        0x00,
        0x84,
        0x5A,
        0x01,
        page >> 8,
        page & 0xFF,
    };

    Serial.write(buff, 10);
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
    u8 buff[] = {
        0x5A,
        0xA5,
        0x05,
        0x82,
        VP >> 8,
        VP & 0xFF,
        data >> 8,
        data & 0xFF,
    };

    Serial.write(buff, 8);
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

    while (Serial.available() < 4)
    {
        if (millis() - startTime > timeout)
            return -1;
        delay(1);
    }

    u8 buff2[4];

    Serial.readBytes(buff2, 4);
    u8 count = buff2[2];

    Serial.println("\nHeader:\n");
    Serial.print("Head 0: ");
    Serial.println(buff2[0], HEX);
    Serial.print("Head 1: ");
    Serial.println(buff2[1], HEX);
    Serial.print("Count: ");
    Serial.println(buff2[2], HEX);
    Serial.print("Code: ");
    Serial.println(buff2[3], HEX);

    while (Serial.available() < count - 1)
    {
        if (millis() - startTime > timeout)
            return -1;
        delay(1);
    }

    short resultLen = Serial.readBytes(buff, count - 1);

    Serial.println("\nData:\n");
    Serial.print("Count: ");
    Serial.println(resultLen);
    Serial.print("VP: ");
    Serial.println(buff[0] << 8 | buff[1], HEX);

    for (int i = 2; i < resultLen; i++)
    {
        Serial.print(i - 2);
        Serial.print(": ");
        Serial.println(buff[i]);
    }

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