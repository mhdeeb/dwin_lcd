#include <EEPROM.h>

void setup()
{
    Serial.begin(9600);

    for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0);
}

void loop() {}