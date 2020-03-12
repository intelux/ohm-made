#include "state.h"

#include "config.h"

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

bool State::fromJsonDocument(const StaticJsonDocument<64> &json)
{
    const String newModeName = json["mode"].as<String>();
    StateMode newMode = StateMode_Count;

    Serial.printf("Parsing state from JSON document for requested mode '%s'\n", newModeName.c_str());

    if (newModeName == "off")
    {
        newMode = StateMode_Off;
    }
    else if (newModeName == "on")
    {
        newMode = StateMode_On;
    }
    else if (newModeName == "pulse")
    {
        newMode = StateMode_Pulse;
    }
    else if (newModeName == "rainbow")
    {
        newMode = StateMode_Rainbow;
    }
    else if (newModeName == "knight-rider")
    {
        newMode = StateMode_KnightRider;
    }
    else if (newModeName == "fire")
    {
        newMode = StateMode_Fire;
    }

    switch (newMode)
    {
    case StateMode_Off:
        break;

    case StateMode_On:
        hue = json["hue"] | hue;
        saturation = json["saturation"] | saturation;
        value = json["value"] | value;
        break;

    case StateMode_Pulse:
        hue = json["hue"] | hue;
        saturation = json["saturation"] | saturation;
        value = json["value"] | value;
        period = json["period"] | period;
        break;

    case StateMode_Rainbow:
        break;

    case StateMode_KnightRider:
        period = json["period"] | period;
        break;

    case StateMode_Fire:
        fire_cooling = json["fire-cooling"] | fire_cooling;
        fire_sparking = json["fire-sparking"] | fire_sparking;
        break;

    default:
        Serial.printf("Ignoring invalid state '%s' from JSON document.", newModeName.c_str());
        return false;
    }

    mode = newMode;

    print();

    return true;
}

void State::cycle()
{
    mode = static_cast<StateMode>(static_cast<int>(mode + 1));

    if (mode >= StateMode_Count)
    {
        mode = StateMode_Off;
    }

    print();
}

void State::print()
{
    switch (mode)
    {
    case StateMode_Off:
        Serial.println("State: off.");
        break;

    case StateMode_On:
        Serial.printf("State: on. HSV: %02x%02x%02x.\n", hue, saturation, value);
        break;

    case StateMode_Pulse:
        Serial.printf("State: pulse. HSV: %02x%02x%02x. Period: %dms\n", hue, saturation, value, period);
        break;

    case StateMode_Rainbow:
        Serial.println("State: rainbow.");
        break;

    case StateMode_KnightRider:
        Serial.printf("State: knight-rider. Period: %dms\n", period);
        break;

    case StateMode_Fire:
        Serial.printf("State: fire. Fire cooling: %d. Fire sparking: %d.\n", fire_cooling, fire_sparking);
        break;

    default:
        Serial.println("State: invalid.");
    }

}

State state;

CRGB leds[MAX_LEDS];

void setupState()
{
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, config.num_leds);
    FastLED.setBrightness(255);
}

void pulse()
{
    int timePosition = millis() % state.period;
    int fadeLevel = (255 * abs(timePosition - state.period / 2) * 2) / state.period;

    fill_solid(leds, config.num_leds, CHSV(state.hue, state.saturation, state.value));
    fadeToBlackBy(leds, config.num_leds, fadeLevel);

    FastLED.show();
}

void rainbow()
{
    fill_rainbow(leds, config.num_leds, 0, 255 / config.num_leds);

    FastLED.show();
}

void knight_rider()
{
    int timePosition = millis() % state.period;
    int position = (config.num_leds * abs(timePosition - state.period / 2) * 2) / state.period;

    for (int i = 0; i < config.num_leds; i++)
    {
        if (i == position)
        {
            leds[i] = CHSV(state.hue, state.saturation, state.value);
        }
        else
        {
            leds[i] = CRGB::Black;
        }
    }

    FastLED.show();
}

void fire()
{
    // Array of temperature readings at each simulation cell
    static byte heat[MAX_LEDS];

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < config.num_leds; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((state.fire_cooling * 10) / config.num_leds) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = config.num_leds - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < state.fire_sparking)
    {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < config.num_leds; j++)
    {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        byte colorindex = scale8(heat[j], 240);
        CRGB color = ColorFromPalette(HeatColors_p, colorindex);

        leds[j] = color;
    }

    FastLED.show();
}

void stateLoop()
{
    random16_add_entropy(analogRead(A0));

    {
        static int lastUpdate = millis();
        // Make sure we don't update fast than necessary.
        const int elapsed = millis() - lastUpdate;
        const int frame_time_left = (1000 / config.fps) - elapsed;

        if (frame_time_left > 0)
        {
            return;
        }

        lastUpdate = millis();
    }

    switch (state.mode)
    {
    case StateMode_Off:
        FastLED.showColor(CRGB::Black);
        break;
    case StateMode_On:
        FastLED.showColor(CHSV(state.hue, state.saturation, state.value));
        break;
    case StateMode_Pulse:
        pulse();
        break;
    case StateMode_Rainbow:
        rainbow();
        break;
    case StateMode_KnightRider:
        knight_rider();
        break;
    case StateMode_Fire:
        fire();
        break;
    default:
        FastLED.showColor(CRGB::Black);
        break;
    }
}