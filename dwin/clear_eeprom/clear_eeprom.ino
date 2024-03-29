#include <EEPROM.h>

void setup()
{
    for (int i = 0; i < EEPROM.length(); i++)
        EEPROM.write(i, 0);

    EEPROM.write(1000, 100);
}

void loop() {}