#include <EEPROM.h>

void setup()
{
    for (int i = 0; i < 511; i++)
        EEPROM.write(i, 0);

    EEPROM.write(511, 50);
}

void loop() {}