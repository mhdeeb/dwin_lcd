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
#define BUTTON_ADVANCED 0x0002
#define BUTTON_STOP 0x0003
#define BUTTON_PAUSE 0x0004
#define BUTTON_EXPORT 0x0005
#define BUTTON_BASIC 0x0006

#define PIN_PUMP 3
#define SD_ChipSelectPin 4
#define EEPROM_RATE 512
#define EEPROM_MODE 513
#define EEPROM_BASIC_TIME_D 514

#define PAGE_WELCOME 0
#define PAGE_START_1 1
#define PAGE_START_2 2
#define PAGE_EDIT 3
#define PAGE_KEYBOARD 4
#define PAGE_WAIT 5
#define PAGE_PAUSE 6
#define PAGE_RUN 7

const u16 DENSITY = 3;
const u16 DEFAULT_RATE = 50;
const u16 PAUSE_TIME = 30;
const u32 WELCOME_DELAY_MS = 5000;

u8 buffer[256]{};
u16 waitTime;
bool isRunning = false;
bool isAdvanced = false;

u16 button;
u8 roomNumber = 0;
u8 roomVolume;
u8 rate;

Timer timer_wait(0);

u16 GetWaitTime(u8 roomVolume)
{
    return (u16)roomVolume * DENSITY * 60 / rate;
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

    for (int i = 0; i < 512; i++)
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

    lcd.ChangePage(PAGE_WELCOME);

    EEPROM.get(EEPROM_RATE, rate);

    lcd.SendData(VP_RATE_EDIT, rate);

    digitalWrite(PIN_PUMP, LOW);

    EEPROM.get(EEPROM_MODE, isAdvanced);

    delay(WELCOME_DELAY_MS);

    if (isAdvanced)
    {
        lcd.SendData(VP_ROOM_NO_START, 000);

        EEPROM.get(000, roomVolume);

        waitTime = GetWaitTime(roomVolume);

        lcd.ChangePage(PAGE_START_2);
    }
    else
    {
        EEPROM.get(EEPROM_BASIC_TIME_D, waitTime);
        waitTime = (waitTime >> 8) * 60 + (waitTime & 0xFF);
        lcd.ChangePage(PAGE_START_1);
    }

    timer_wait.Set(waitTime);
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
            case BUTTON_ADVANCED:
                isAdvanced = true;

                EEPROM.put(EEPROM_MODE, isAdvanced);

                lcd.SendData(VP_ROOM_NO_START, roomNumber);

                EEPROM.get(roomNumber, roomVolume);

                waitTime = GetWaitTime(roomVolume);

                timer_wait.Set(waitTime);
                break;
            case BUTTON_STOP:
                isRunning = false;

                timer_wait.Stop();

                timer_wait.Set(waitTime);

                digitalWrite(PIN_PUMP, LOW);

                lcd.ChangePage(isAdvanced ? PAGE_START_2 : PAGE_START_1);
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
                timer_wait.Set(PAUSE_TIME);
                timer_wait.Start();
                break;
            case BUTTON_BASIC:
                isAdvanced = false;

                EEPROM.put(EEPROM_MODE, isAdvanced);

                EEPROM.get(EEPROM_BASIC_TIME_D, waitTime);

                waitTime = (waitTime >> 8) * 60 + (waitTime & 0xFF);

                timer_wait.Set(waitTime);
                break;
            case BUTTON_EDIT:
                EEPROM.get(roomNumber, roomVolume);

                lcd.SendData(VP_ROOM_NO_EDIT, roomNumber);

                lcd.SendData(VP_ROOM_VOL_EDIT, roomVolume);

                EEPROM.get(EEPROM_RATE, rate);

                lcd.SendData(VP_RATE_EDIT, rate);
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
                buffer[4] = DEFAULT_RATE;
                lcd.SendData(VP_RATE_EDIT, buffer[4]);
            }

            rate = buffer[4];

            EEPROM.put(EEPROM_RATE, rate);
            break;
        case VP_TIMER:
            EEPROM.put(EEPROM_BASIC_TIME_D, buffer[4]);

            EEPROM.put(EEPROM_BASIC_TIME_D + 1, buffer[3]);

            waitTime = buffer[3] * 60 + buffer[4];

            timer_wait.Set(waitTime);

            timer_wait.PopChanged();
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
            lcd.ChangePage(isAdvanced ? PAGE_START_2 : PAGE_START_1);
            digitalWrite(PIN_PUMP, LOW);
            isRunning = false;
        }
        else
        {
            timer_wait.Start();
            lcd.ChangePage(PAGE_RUN);
            digitalWrite(PIN_PUMP, HIGH);
            isRunning = true;
        }
    }
}