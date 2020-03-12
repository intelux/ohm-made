#pragma once

#include <cstdint>

#include <ArduinoJson.h>

enum StateMode
{
    StateMode_Off = 0,
    StateMode_On = 1,
    StateMode_Pulse = 2,
    StateMode_Rainbow = 3,
    StateMode_KnightRider = 4,
    StateMode_Fire = 5,
    StateMode_Count,
};

class State
{
public:
    bool fromJsonDocument(const StaticJsonDocument<256>& json);
    void toJsonDocument(StaticJsonDocument<256> &json);
    void cycle();
    void print();

    StateMode mode = StateMode_Off;
    uint8_t hue = 0;
    uint8_t saturation = 0;
    uint8_t value = 255;
    uint32_t period = 5000;
    uint8_t fire_cooling = 40;
    uint8_t fire_sparking = 80;
};

extern State state;

void setupState();
void stateLoop();
void cycleState();