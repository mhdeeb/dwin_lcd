#include "dwin_lcd.h"
#include <EEPROM.h>

DwinLCD lcd(9600, 0, 1);

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

void setup()
{
  Serial.begin(9600);

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
  denisty = EEPROM.read(1) << 8;
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
        Serial.println("Start");
        break;
      case 2:
        Serial.println("Stop Wait");
        break;
      case 3:
        Serial.println("Stop Clean");
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
}