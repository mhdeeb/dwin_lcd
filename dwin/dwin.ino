#include "dwin_lcd.h"
#include "Timer.h"

DwinLCD lcd;

#define VP_BUTTON_RESET 0x1001
#define VP_BUTTON_PAUSE 0x1002
#define VP_BUTTON_START 0x1003
#define VP_WAIT_TIME 0x1004

#define PIN_PUMP 3

u8 buffer[256]{};
u16 waitTime = 300;

Timer timer_wait(waitTime);

void setup()
{
    lcd.being(9600);

    lcd.SendData(VP_WAIT_TIME, waitTime);
}

void loop()
{
    short size = lcd.ReadData(buffer, 256, 1);

    if (size != -1)
    {
        u16 VP = (u16)buffer[0] << 8 | buffer[1];
        switch (VP)
        {
        case VP_BUTTON_RESET:
            timer_wait.Reset();
            break;
        case VP_BUTTON_PAUSE:
            timer_wait.Stop();
            break;
        case VP_BUTTON_START:
            timer_wait.Start();
            break;
        case VP_WAIT_TIME:
            waitTime = (u16)buffer[2] << 8 | buffer[3];
            timer_wait.Set(waitTime);
            break;
        }
    }

    timer_wait.Update();

    if (timer_wait.IsRunning())
    {
        digitalWrite(PIN_PUMP, HIGH);
    }
    else
    {
        digitalWrite(PIN_PUMP, LOW);
    }

    u16 timer = timer_wait.GetTime();

    lcd.SendData(VP_WAIT_TIME, timer / 60 << 8 | timer % 60);
}