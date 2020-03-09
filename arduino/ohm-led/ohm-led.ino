#include <ESP_EEPROM.h>

#include <ArduinoJson.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

#define MAX_LEDS 128
#define DEFAULT_NUM_LEDS 16

struct LEDsConfig {
  bool configured = false;
  uint16_t numLEDs = DEFAULT_NUM_LEDS;

  void configuredOrDefaults() {
    if (!configured) {
      *this = {};
    }
  }
} ledsConfig;

CRGB leds[MAX_LEDS];

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>

#define AP_SSID "ohm-led"
#define AP_PASSPHRASE "password"
#define DEFAULT_HTTP_PORT 80

struct WiFiConfig {
  bool configured = false;
  char ssid[64] = {};
  char passphrase[64] = {};
  uint16_t httpPort = DEFAULT_HTTP_PORT;

  void configuredOrDefaults() {
    if (!configured) {
      *this = {};
    }
  }
} wiFiConfig;

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server;

void handleGetStatus();
void handleSetStatus();
void handleNotFound();

#define LEDS_DATA_PIN 1
#define EXTERNAL_LED_PIN D3
#define BUTTON_PIN D5

void setup(void){
  Serial.begin(74880);
  delay(1000);

  Serial.println();
  Serial.println("Initializing...");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(EXTERNAL_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  EEPROM.begin(sizeof(ledsConfig) + sizeof(wiFiConfig));
  EEPROM.get(0, ledsConfig);
  EEPROM.get(sizeof(ledsConfig), wiFiConfig);

  ledsConfig.configuredOrDefaults();
  wiFiConfig.configuredOrDefaults();

  FastLED.addLeds<WS2812, LEDS_DATA_PIN, GRB>(leds, ledsConfig.numLEDs);
  Serial.printf("Controller has %d led(s).\n", ledsConfig.numLEDs);

  Serial.println("Initializing WiFi...");

  if (!wiFiConfig.configured) {
    Serial.println("WiFi has never been configured. Starting in Access-Point mode...");

    IPAddress ip(192, 168, 16, 1);
    IPAddress gateway(192, 168, 16, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);

    uint8_t mode = 0;
    wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &mode);

    if (WiFi.softAP(AP_SSID, AP_PASSPHRASE)) {
      Serial.printf("Access-Point SSID: %s\n", AP_SSID);
      Serial.printf("Access-Point passphrase: %s\n", AP_PASSPHRASE);
      Serial.printf("Access-Point IP address: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
      Serial.println("Failed to start Access-Point! Something might be wrong with the chip.");
    }
  } else {
    Serial.println("WiFi has been configured. Starting in client mode...");

    wifiMulti.addAP(wiFiConfig.ssid, wiFiConfig.passphrase);
    Serial.printf("Connecting to '%s'...\n", wiFiConfig.ssid);

    while (wifiMulti.run() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(EXTERNAL_LED_PIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(EXTERNAL_LED_PIN, LOW);
    }

    Serial.printf("Connected to %s.\n", wiFiConfig.ssid);
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }

  server.on("/v1/status/", HTTP_GET, handleGetStatus);
  server.on("/v1/status/", HTTP_PUT, handleSetStatus);
  server.onNotFound(handleNotFound);

  const char* headerkeys[] = {"content-type"};
  server.collectHeaders(headerkeys, sizeof(headerkeys)/sizeof(char*));
  server.begin(wiFiConfig.httpPort);

  Serial.printf("HTTP server started on port %d.\n", wiFiConfig.httpPort);
}

void loop(void){
  server.handleClient();

  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(EXTERNAL_LED_PIN, LOW);
    FastLED.showColor(CRGB::Red);
  } else {
    digitalWrite(EXTERNAL_LED_PIN, HIGH);
    FastLED.showColor(CRGB::Green);
  }
}

void handleGetStatus() {
  int v = analogRead(A0);
  char tmp[16];
  snprintf(tmp, 16, "{\"value\": %0.2f}", v / 1024.0f);
  server.send(200, "application/json", tmp);
}

void handleSetStatus() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing message body.\n");
    return;
  }

  const String contentType = server.header("content-type");

  if (contentType != "application/json") {
    char tmp[128];
    snprintf(tmp, 128, "Expecting 'application/json' content-type, got: '%s'.\n", contentType.c_str());
    server.send(400, "text/plain", tmp);
    return;
  }

  StaticJsonDocument<64> doc;

  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    char tmp[128];
    snprintf(tmp, 128, "JSON error: %s", error.c_str());
    server.send(400, "text/plain", tmp);
    return;
  }

  static uint8_t hue = 0;
  FastLED.showColor(CHSV(hue+= 32, 255, 255));

  server.send(200, "application/json", "{}\n");
}

void handleNotFound(){
  server.send(404, "text/plain", "Not found.\n");
}
