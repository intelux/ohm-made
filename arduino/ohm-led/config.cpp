#include "config.h"

#include <ESP_EEPROM.h>

Config config;

bool Config::Load()
{
    bool result = true;

    EEPROM.begin(sizeof(*this));
    EEPROM.get(0, *this);

    if (!isValid())
    {
        result = false;
        *this = Config();
    }

    EEPROM.end();

    return result;
}

bool Config::Save() const
{
    // Make sure we write the magic number.
    magic_value = MAGIC_VALUE;

    EEPROM.begin(sizeof(*this));
    EEPROM.put(0, *this);
    EEPROM.commit();
    EEPROM.end();

    return true;
}