#include <EEPROM.h>

void setup()
{
    for (int i = 0; i < 516; i++)
        EEPROM.write(i, 0);

    EEPROM.write(512, 50);
}

void loop() {}