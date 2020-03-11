#include "state.h"

#include "config.h"

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

bool State::FromJsonDocument(const StaticJsonDocument<64> &json)
{
    const StateMode newMode = static_cast<StateMode>(json["mode"].as<int>());

    if ((newMode < StateMode_Off) || (newMode >= StateMode_Count))
    {
        return false;
    }

    mode = newMode;

    return true;
}

State state;

CRGB leds[MAX_LEDS];

void setupState()
{
    FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, config.num_leds);
    FastLED.setBrightness(255);
}

// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING 40

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 80

void fire()
{
    // Array of temperature readings at each simulation cell
    static byte heat[MAX_LEDS];

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < config.num_leds; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / config.num_leds) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = config.num_leds - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKING)
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
}

void stateLoop()
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

    switch (state.mode)
    {
    case StateMode_Off:
        FastLED.showColor(CRGB::Black);
        break;
    case StateMode_On:
        FastLED.showColor(CHSV(state.hue, state.saturation, state.value));
        break;
    case StateMode_Rainbow:
        fill_rainbow(leds, config.num_leds, 0);
        FastLED.show();
        break;
    case StateMode_Fire:
        fire();
        FastLED.show();
        break;
    default:
        FastLED.showColor(CRGB::Black);
        break;
    }
}