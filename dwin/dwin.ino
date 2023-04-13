#include "dwin_lcd.h"

DwinLCD lcd(9600, A5, A4);

#define VP_BUTTON 0x1001
#define VP_ROOM_VOLUME 0x1002
#define VP_WAIT_TIME_M 0x1003
#define VP_WAIT_TIME_S 0x1004
#define VP_DENISTY 0x1005
#define VP_TIMER_PREVIEW 0x1006
#define VP_TIMER_WAIT 0x1008
#define VP_TIMER_CLEAN 0x100A

#define PIN_PUMP 3

u8 buffer[256]{};
u8 previewTimer[2]{};
u8 button{};
u8 VU{};

void setup()
{
  Serial.begin(9600);
  lcd.SendData(VP_BUTTON, 0x00);
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
      Serial.println("Room Volume");
      lcd.ReadData(VP_TIMER_PREVIEW, previewTimer, 2, 1000);
      break;
    case VP_WAIT_TIME_M:
      Serial.println("Wait Time M");
      break;
    case VP_WAIT_TIME_S:
      Serial.println("Wait Time S");
      break;
    case VP_DENISTY:
      Serial.println("Denisty");
      break;
    }
  }
}