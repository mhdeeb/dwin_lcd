#include <EEPROM.h>

void setup()
{
    for (int i = 0; i < 503; i++)
        EEPROM.put(i * 2, 0);

    EEPROM.put(1000, 50 << 8);
}

void loop() {}