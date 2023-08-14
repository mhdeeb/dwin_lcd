#include "dwin_lcd.h"
#include "Timer.h"

DwinLCD lcd;

#define VP_ROOM_NO_START 0x1001
#define VP_ROOM_NO_EDIT 0x1004
#define VP_ROOM_VOL_EDIT 0x1007

#define VP_BUTTONS 0x100B
#define VP_TIMER 0x100C

#define BUTTON_EDIT 0x0000
#define BUTTON_START 0x0001
#define BUTTON_RETURN 0x0002
#define BUTTON_STOP 0x0003
#define BUTTON_PAUSE 0x0004

#define PIN_PUMP 3

u8 buffer[256]{};
u16 waitTime = 300;
const u16 pauseTime = 30;
u16 lastTime = waitTime;
bool isRunning = false;

Timer timer_wait(waitTime);

void setup()
{
    lcd.being(9600);
    lcd.ChangePage(0);
    digitalWrite(PIN_PUMP, LOW);
    Serial.println("POWER ON");
    lcd.SendData(VP_TIMER, 48);
    u16 p;
    Serial.println(lcd.ReadPointer(VP_TIMER, p));
    Serial.println(p);
}

void loop()
{
    short size = lcd.ReadData(buffer, 256, 1000);

    if (size != -1)
    {
        u16 VP = (u16)buffer[0] << 8 | buffer[1];
        u16 button = buffer[4];
        switch (VP)
        {
        case VP_BUTTONS:
            switch (button)
            {
            case BUTTON_STOP:
                timer_wait.Reset();
                if (!isRunning)
                {
                    timer_wait.Set(lastTime);
                }
                else
                {
                    digitalWrite(PIN_PUMP, LOW);
                    Serial.println("PUMP STOPPED");
                }
                break;
            case BUTTON_PAUSE:
                if (timer_wait.IsRunning())
                {
                    timer_wait.Stop();
                    digitalWrite(PIN_PUMP, LOW);
                    Serial.println("PUMP PAUSED");
                }
                else
                {
                    timer_wait.Start();
                    digitalWrite(PIN_PUMP, HIGH);
                    Serial.println("PUMP RESUMED");
                }
                break;
            case BUTTON_START:
            {
                lastTime = timer_wait.GetTime();
                timer_wait.Set(pauseTime);
                timer_wait.Start();
            }
            break;
            }
            break;
        case VP_ROOM_NO_START:
            waitTime = buffer[3] * 60 + buffer[4];
            lastTime = waitTime;
            timer_wait.Set(waitTime);
            timer_wait.PopChanged();
            break;
        case VP_ROOM_NO_EDIT:
            waitTime = buffer[3] * 60 + buffer[4];
            lastTime = waitTime;
            timer_wait.Set(waitTime);
            timer_wait.PopChanged();
            break;
        case VP_ROOM_VOL_EDIT:
            waitTime = buffer[3] * 60 + buffer[4];
            lastTime = waitTime;
            timer_wait.Set(waitTime);
            timer_wait.PopChanged();
            break;
        case VP_TIMER:
            waitTime = buffer[3] * 60 + buffer[4];
            lastTime = waitTime;
            timer_wait.Set(waitTime);
            timer_wait.PopChanged();
            break;
        }
    }

    timer_wait.Update();

    if (timer_wait.PopChanged())
    {
        u16 currTime = timer_wait.GetTime();
        //lcd.SendData(VP_TIMER, currTime / 60 << 8 | currTime % 60);
    }

    if (timer_wait.IsFinished())
    {
        if (isRunning)
        {
            lcd.ChangePage(0);
            timer_wait.Reset();
            isRunning = false;
            digitalWrite(PIN_PUMP, LOW);
            Serial.println("PUMP FINISHED");
        }
        else
        {
            lcd.ChangePage(4);
            timer_wait.Set(lastTime);
            timer_wait.Start();
            isRunning = true;
            digitalWrite(PIN_PUMP, HIGH);
            Serial.println("PUMP STARTED");
        }
    }
}