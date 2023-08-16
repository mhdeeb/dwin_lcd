#include "dwin_lcd.h"
#include "Timer.h"

#include <EEPROM.h>
#include <SD.h>

DwinLCD lcd;

#define VP_ROOM_NO_START 0x1001
#define VP_ROOM_NO_EDIT 0x1004
#define VP_ROOM_VOL_EDIT 0x1005
#define VP_RATE_EDIT 0x1006

#define VP_BUTTONS 0x100B
#define VP_TIMER 0x100C

#define BUTTON_EDIT 0x0000
#define BUTTON_START 0x0001
#define BUTTON_RETURN 0x0002
#define BUTTON_STOP 0x0003
#define BUTTON_PAUSE 0x0004
#define BUTTON_EXPORT 0x0005

#define PIN_PUMP 3
#define SD_ChipSelectPin 4
#define RATE_ADDRESS 512

const u16 density = 3;
const u16 defaultRate = 50;
const u16 pauseTime = 30;

u8 buffer[256]{};
u16 waitTime;
bool isRunning = false;

u16 button;
u8 roomNumber;
u8 roomVolume;
u8 rate;

Timer timer_wait(0);

u16 GetWaitTime(u8 roomVolume)
{
    return (u16)roomVolume * density * 60 / rate;
}

void saveData()
{
    if (!SD.begin(SD_ChipSelectPin))
    {
        return;
    }

    if (SD.exists("data.csv"))
        SD.remove("data.csv");

    File dataFile = SD.open("data.csv", FILE_WRITE);

    dataFile.println("Room No.,Room Vol.,Run Time");

    u8 rv;

    for (int i = 0; i < 511; i++)
    {
        EEPROM.get(i, rv);
        if (!rv)
            continue;
        dataFile.print(i);
        dataFile.print(",");
        dataFile.print(rv);
        dataFile.print(",");
        dataFile.println(GetWaitTime(rv));
    }

    dataFile.close();
}

void setup()
{
    pinMode(PIN_PUMP, OUTPUT);

    lcd.being(9600);

    lcd.ChangePage(0);

    lcd.SendData(VP_ROOM_NO_START, 000);

    EEPROM.get(000, roomVolume);

    EEPROM.get(RATE_ADDRESS, rate);

    lcd.SendData(VP_RATE_EDIT, rate);

    waitTime = GetWaitTime(roomVolume);

    timer_wait.Set(waitTime);

    digitalWrite(PIN_PUMP, LOW);
}

void loop()
{
    short size = lcd.ReadData(buffer, 256, 1000);

    if (size != -1)
    {
        u16 VP = (u16)buffer[0] << 8 | buffer[1];

        switch (VP)
        {
        case VP_BUTTONS:
            button = buffer[4];
            switch (button)
            {
            case BUTTON_EXPORT:
                saveData();
                break;
            case BUTTON_RETURN:
                lcd.SendData(VP_ROOM_NO_START, 000);

                EEPROM.get(000, roomVolume);

                waitTime = GetWaitTime(roomVolume);

                timer_wait.Set(waitTime);
                break;
            case BUTTON_STOP:
                isRunning = false;
                timer_wait.Stop();
                timer_wait.Set(waitTime);
                digitalWrite(PIN_PUMP, LOW);
                break;
            case BUTTON_PAUSE:
                if (timer_wait.IsRunning())
                {
                    timer_wait.Stop();
                    digitalWrite(PIN_PUMP, LOW);
                }
                else
                {
                    timer_wait.Start();
                    digitalWrite(PIN_PUMP, HIGH);
                }
                break;
            case BUTTON_START:
            {
                timer_wait.Set(pauseTime);
                timer_wait.Start();
            }
            break;
            }
            break;
        case VP_ROOM_NO_START:
            roomNumber = buffer[4];

            EEPROM.get(roomNumber, roomVolume);

            waitTime = GetWaitTime(roomVolume);

            timer_wait.Set(waitTime);
            break;
        case VP_ROOM_NO_EDIT:
            roomNumber = buffer[4];

            EEPROM.get(roomNumber, roomVolume);

            lcd.SendData(VP_ROOM_VOL_EDIT, roomVolume);
            break;
        case VP_ROOM_VOL_EDIT:
            roomVolume = buffer[4];

            EEPROM.put(roomNumber, roomVolume);
            break;
        case VP_RATE_EDIT:
            if (!buffer[4])
            {
                buffer[4] = defaultRate;
                lcd.SendData(VP_RATE_EDIT, buffer[4]);
            }

            rate = buffer[4];

            EEPROM.put(RATE_ADDRESS, rate);
            break;
        }
    }

    timer_wait.Update();

    if (timer_wait.PopChanged())
    {
        u16 currTime = timer_wait.GetTime();
        lcd.SendData(VP_TIMER, currTime / 60 << 8 | currTime % 60);
    }

    if (timer_wait.IsFinished())
    {
        timer_wait.Stop();
        timer_wait.Set(waitTime);
        if (isRunning)
        {
            lcd.ChangePage(0);
            isRunning = false;
            digitalWrite(PIN_PUMP, LOW);
        }
        else
        {
            timer_wait.Start();
            lcd.ChangePage(4);
            isRunning = true;
            digitalWrite(PIN_PUMP, HIGH);
        }
    }
}