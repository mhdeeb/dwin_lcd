#include "dwin_lcd.h"
#include "Timer.h"

#include <EEPROM.h>

DwinLCD lcd;

#define VP_ROOM_VOLUME 0x1001
#define VP_TIMER_PREVIEW 0x1002
#define VP_WAIT_TIME 0x1003
#define VP_DENISTY 0x1004
#define VP_TIMER_WAIT 0x1005
#define VP_TIMER_CLEAN 0x1006
#define VP_BUTTON 0x1007

#define PIN_PUMP 3

u8 buffer[256]{};
u16 previewTimer;
u8 button{};
u8 VU{};
u16 waitTime;
u16 denisty;
u16 roomVolume = 1;
u8 pumpPerSec = 2;

Timer timer_wait(0);
Timer timer_work(0);

void setup()
{
  lcd.being(9600);

  lcd.SendData(VP_BUTTON, 0x00);
  lcd.SendData(VP_ROOM_VOLUME, roomVolume << 8);

  if (EEPROM.read(0) != 0xFF)
  {
    EEPROM.write(0, 0xFF);
    EEPROM.write(1, 0x01);
    EEPROM.write(2, 0x00);
    EEPROM.write(3, 0x1E);
  }

  lcd.SendData(VP_WAIT_TIME, EEPROM.read(2) << 8 | EEPROM.read(3));
  lcd.SendData(VP_DENISTY, EEPROM.read(1) << 8);

  waitTime = EEPROM.read(2) << 8 | EEPROM.read(3);
  denisty = EEPROM.read(1) << 8;

  timer_wait.Set(waitTime);
}

void loop()
{
  short size = lcd.ReadData(buffer, 256);

  if (size != -1)
  {
    u16 VP = (u16)buffer[4] << 8 | buffer[5];
    switch (VP)
    {
    case VP_BUTTON:
      button = buffer[6];
      switch (button)
      {
      case 1:
        timer_wait.Set(waitTime);
        timer_wait.Start();
        break;
      case 2:
        lcd.ChangePage(2);
        timer_wait.Reset();
        break;
      case 3:
        lcd.ChangePage(2);
        timer_work.Reset();
        break;
      }
      button = 0;
      break;
    case VP_ROOM_VOLUME:
      lcd.ReadPointer(VP_ROOM_VOLUME, roomVolume);
      break;
    case VP_WAIT_TIME:
      if (lcd.ReadPointer(VP_WAIT_TIME, waitTime))
      {
        EEPROM.write(2, waitTime >> 8);
        EEPROM.write(3, waitTime & 0xFF);
      }
      break;
    case VP_DENISTY:
      if (lcd.ReadPointer(VP_DENISTY, denisty))
      {
        EEPROM.write(1, denisty >> 8);
      }
      break;
    }

    previewTimer = denisty * roomVolume / pumpPerSec;

    lcd.SendData(VP_TIMER_PREVIEW, previewTimer);
  }

  timer_wait.Update();
  timer_work.Update();

  if (timer_wait.IsFinished())
  {
    timer_wait.Reset();
    lcd.ChangePage(4);
    digitalWrite(PIN_PUMP, HIGH);
  }

  if (timer_work.IsFinished())
  {
    timer_work.Reset();
    lcd.ChangePage(0);
    digitalWrite(PIN_PUMP, LOW);
  }

  u16 timer = timer_wait.GetTime() / 1000;

  lcd.SendData(VP_TIMER_WAIT, timer / 60 << 8 | timer % 60);

  timer = timer_work.GetTime() / 1000;

  lcd.SendData(VP_TIMER_CLEAN, timer / 60 << 8 | timer % 60);
}
