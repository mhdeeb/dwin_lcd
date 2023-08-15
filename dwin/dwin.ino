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
const u16 pauseTime = 30;
bool isRunning = false;

Timer timer_wait(waitTime);

void setup()
{
    lcd.being(9600);
    lcd.ChangePage(0);
    pinMode(PIN_PUMP, OUTPUT);
    digitalWrite(PIN_PUMP, LOW);
    Serial.println("POWER ON");
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
            timer_wait.Stop();
            timer_wait.Set(waitTime);
            isRunning = false;
            digitalWrite(PIN_PUMP, LOW);
            break;
        case VP_BUTTON_PAUSE:
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
        case VP_BUTTON_START:
            timer_wait.Set(pauseTime);
            timer_wait.Start();
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
            lcd.ChangePage(1);
            timer_wait.Start();
            isRunning = true;
            digitalWrite(PIN_PUMP, HIGH);
        }
    }
}