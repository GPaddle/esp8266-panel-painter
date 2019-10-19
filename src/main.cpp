
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include "config.h"

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
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      // TODO: handle payload via ArduinoJSON
      break;
    case WStype_CONNECTED:
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
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

  Serial.println("Ready");
}

void loop() {
  webSocket.loop();
  server.handleClient();
}