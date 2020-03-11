#include "config.h"
#include "web.h"

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

CRGB leds[MAX_LEDS];

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>

ESP8266WiFiMulti wifiMulti;

void handleGetStatus();
void handleSetStatus();
void handleNotFound();

#define LEDS_DATA_PIN 1
#define EXTERNAL_LED_PIN D3
#define BUTTON_PIN D5

void setup(void)
{
  Serial.begin(74880);
  delay(1000);

  Serial.println();
  Serial.println(F("Initializing..."));

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(EXTERNAL_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  if (!config.Load())
  {
    Serial.println(F("No existing configuration was found. Assuming default configuration."));
  }
  else
  {
    Serial.println(F("Loaded existing configuration."));
  }

  FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, config.num_leds);
  Serial.printf("Controller has %d led(s).\n", config.num_leds);

  Serial.println(F("Initializing WiFi..."));

  if (!config.hasSSID())
  {
    Serial.println(F("WiFi has never been configured. Starting in Access-Point mode..."));

    IPAddress ip(192, 168, 16, 1);
    IPAddress gateway(192, 168, 16, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);

    uint8_t mode = 0;
    wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);

    if (WiFi.softAP(AP_SSID, AP_PASSPHRASE))
    {
      Serial.printf("Access-Point SSID: %s\n", AP_SSID);
      Serial.printf("Access-Point passphrase: %s\n", AP_PASSPHRASE);
      Serial.printf("Access-Point IP address: %s\n", WiFi.softAPIP().toString().c_str());
    }
    else
    {
      Serial.println(F("Failed to start Access-Point! Something might be wrong with the chip."));
    }
  }
  else
  {
    Serial.println(F("WiFi has been configured. Starting in client mode..."));

    wifiMulti.addAP(config.ssid, config.passphrase);
    Serial.printf("Connecting to '%s'...\n", config.ssid);

    while (wifiMulti.run() != WL_CONNECTED)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(EXTERNAL_LED_PIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(EXTERNAL_LED_PIN, LOW);
    }

    Serial.printf("Connected to '%s'.\n", config.ssid);
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }

  startWebServer(config.http_port);

  Serial.printf("HTTP server started on port %d.\n", config.http_port);

  if (config.hasName())
  {
    Serial.printf("Using configured name '%s' as a hostname.\n", config.name);
    WiFi.hostname(config.name);

    if (MDNS.begin(config.name, WiFi.localIP()))
    {
      Serial.println(F("MDNS has been set up."));
      MDNS.addService("http", "tcp", config.http_port);
      MDNS.addService("ohm-led", "tcp", config.http_port);
    } else {
      Serial.println(F("Failed to setup MDNS."));
    }
  }
}

int pressedTime = 0;

void loop(void)
{
  webServerLoop();
  MDNS.update();

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    int pressedDuration = millis() - pressedTime;

    if (pressedDuration < 2000)
    {
      if ((pressedDuration / 500) % 2 == 0)
      {
        digitalWrite(EXTERNAL_LED_PIN, LOW);
      }
      else
      {
        digitalWrite(EXTERNAL_LED_PIN, HIGH);
      }

      FastLED.showColor(CRGB::OrangeRed);
    }
    else if (pressedDuration < 4000)
    {
      if ((pressedDuration / 250) % 2 == 0)
      {
        digitalWrite(EXTERNAL_LED_PIN, LOW);
      }
      else
      {
        digitalWrite(EXTERNAL_LED_PIN, HIGH);
      }

      FastLED.showColor(CRGB::Orange);
    }
    else if (pressedDuration < 6000)
    {
      if ((pressedDuration / 125) % 2 == 0)
      {
        digitalWrite(EXTERNAL_LED_PIN, LOW);
      }
      else
      {
        digitalWrite(EXTERNAL_LED_PIN, HIGH);
      }

      FastLED.showColor(CRGB::Yellow);
    }
    else if (pressedDuration < 8000)
    {
      if ((pressedDuration / 67) % 2 == 0)
      {
        digitalWrite(EXTERNAL_LED_PIN, LOW);
      }
      else
      {
        digitalWrite(EXTERNAL_LED_PIN, HIGH);
      }

      FastLED.showColor(CRGB::YellowGreen);
    }
    else
    {
      digitalWrite(EXTERNAL_LED_PIN, HIGH);
      FastLED.showColor(CRGB::Green);
    }
  }
  else
  {
    pressedTime = millis();
    digitalWrite(EXTERNAL_LED_PIN, LOW);
    FastLED.showColor(CRGB::Red);
  }
}
