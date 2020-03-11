#pragma once

#include <cstdint>

#include <ArduinoJson.h>

enum StateMode
{
    StateMode_Off = 0,
    StateMode_On = 1,
    StateMode_Rainbow = 2,
    StateMode_Fire = 3,
    StateMode_Count,
};

class State
{
public:
    bool FromJsonDocument(const StaticJsonDocument<64>& json);

    StateMode mode = StateMode_Off;
    uint8_t hue = 0;
    uint8_t saturation = 255;
    uint8_t value = 255;
    uint32_t speed = 1;
};

extern State state;

void setupState();
void stateLoop();