
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include "config.h"

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#define LED_BRIGHTNESS 32
const uint8_t matrixWidth = 32;
const uint8_t matrixHeight = 8;
CRGB leds[matrixWidth * matrixHeight];

#define serve(server, uri, filePath, contentType)             \
  {                                                           \
    server.on(uri, [&]() {                                    \
      File file = SPIFFS.open(filePath, "r");                 \
      int sentBytes = server.streamFile(file, contentType);   \
      Serial.println(String("GET ") + uri + " " + sentBytes); \
      file.close();                                           \
    });                                                       \
  }

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);

const size_t capacity = JSON_ARRAY_SIZE(5);
DynamicJsonDocument doc(capacity);

void waitForWifi() {
  WiFi.hostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  do {
    Serial.println(String("Connecting to ") + WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(4000);
  } while (WiFi.status() != WL_CONNECTED);
}

void handleWebSocket(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght) {
  int x, y, r, g, b;
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      // TODO: handle payload via ArduinoJSON
      deserializeJson(doc, payload);
      x = doc[0];  // 20
      y = doc[1];  // 3
      r = doc[2];  // 255
      g = doc[3];  // 0
      b = doc[4];  // 0

      if (x % 2) {
        // uneven columns
        leds[(matrixWidth * matrixHeight - 1) - (x * matrixHeight + y)] = CRGB(r, g, b);
      } else {
        // even columns
        leds[(matrixWidth * matrixHeight - 1) - (x * matrixHeight + ((matrixHeight - 1) - y))] = CRGB(r, g, b);
      }

      LEDS.show();
      break;
    case WStype_CONNECTED:
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
  }
}

void initLeds() {
  for (int y = 0; y < matrixHeight; y++) {
    for (int x = 0; x < matrixWidth; x++) {
      leds[y * matrixWidth + x] = CRGB(255, 255, 255);
    }
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Starting...");

  SPIFFS.begin();
  Serial.println("Spiffs started");

  waitForWifi();
  Serial.println("WiFi started");

  serve(server, "/", "/index.html.gz", "text/html");
  serve(server, "/index.html", "/index.html.gz", "text/html");
  serve(server, "/style.css", "/style.css.gz", "test/css");
  serve(server, "/app.js", "/app.js.gz", "application/javascript");
  serve(server, "/p5.min.js", "/p5.min.js.gz", "application/javascript");
  serve(server, "/addons/p5.dom.min.js", "/addons/p5.dom.min.js.gz", "application/javascript");
  serve(server, "/addons/p5.sound.min.js", "/addons/p5.sound.min.js.gz", "application/javascript");
  server.begin();
  Serial.println("HTTP Server started");

  webSocket.begin();
  webSocket.onEvent(handleWebSocket);
  Serial.println("Websocket Server started");

  initLeds();
  LEDS.addLeds<WS2812, 4, GRB>(leds, matrixWidth * matrixHeight);
  LEDS.setBrightness(LED_BRIGHTNESS);
  Serial.println("Ready");
}

void loop() {
  webSocket.loop();
  server.handleClient();
}