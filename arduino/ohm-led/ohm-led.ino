#include "config.h"

#include <ArduinoJson.h>

#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

CRGB leds[NUM_LEDS];

#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(HTTP_PORT);

void handleGetStatus();
void handleSetStatus();
void handleNotFound();

void setup(void){
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Initializing...");

  FastLED.addLeds<WS2812, 1, GRB>(leds, NUM_LEDS);
  Serial.printf("Controller has %d led(s).\n", NUM_LEDS);

  wifiMulti.addAP(WIFI_SSID, WIFI_PASSPHRASE);

  int i = 0;
  
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Initializing WiFi...");
  Serial.printf("Connecting to " WIFI_SSID "...\n");

  while (wifiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.printf("Connected to %s.\n", WIFI_SSID);
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  server.on("/status/", HTTP_GET, handleGetStatus);
  server.on("/status/", HTTP_PUT, handleSetStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  
  Serial.printf("HTTP server started on port %d.\n", HTTP_PORT);
}

void loop(void){
  server.handleClient();
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
    snprintf(tmp, 128, "Expecting 'application/json' content-type, got: %s.\n", contentType.c_str());
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
