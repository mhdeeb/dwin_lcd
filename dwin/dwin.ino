#include "dwin_lcd.h"
#include "Timer.h"

DwinLCD lcd;

#define VP_BUTTON_RESET 0x1001
#define VP_BUTTON_PAUSE 0x1002
#define VP_BUTTON_START 0x1003
#define VP_WAIT_TIME 0x1004

#define PIN_PUMP 3

u8 buffer[256]{};
u16 waitTime = 10;
u16 pauseTime = 5;
u16 lastTime = 0;
bool isRunning = false;

Timer timer_wait(waitTime);

void setup()
{
    lcd.being(9600);
}

void loop()
{
    short size = lcd.ReadData(buffer, 256, 1000);

    if (size != -1)
    {
        u16 VP = (u16)buffer[0] << 8 | buffer[1];
        switch (VP)
        {
        case VP_BUTTON_RESET:
            timer_wait.Reset();
            if (!isRunning)
            {
                timer_wait.Set(lastTime);
            }
            break;
        case VP_BUTTON_PAUSE:
            if (timer_wait.IsRunning())
                timer_wait.Stop();
            else
                timer_wait.Start();
            break;
        case VP_BUTTON_START:
        {
            lastTime = timer_wait.GetTime();
            timer_wait.Set(pauseTime);
            timer_wait.Start();
        }
        break;
        case VP_WAIT_TIME:
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
        lcd.SendData(VP_WAIT_TIME, currTime / 60 << 8 | currTime % 60);
    }

    if (timer_wait.IsRunning() && isRunning)
    {
        digitalWrite(PIN_PUMP, HIGH);
        Serial.println("PUMP ON");
    }
    else
    {
        digitalWrite(PIN_PUMP, LOW);
        Serial.println("PUMP OFF");
    }

    if (timer_wait.IsFinished())
    {
        if (isRunning)
        {
            lcd.ChangePage(0);
            timer_wait.Reset();
            isRunning = false;
        }
        else
        {
            lcd.ChangePage(1);
            timer_wait.Set(lastTime);
            timer_wait.Start();
            isRunning = true;
        }
    }
}