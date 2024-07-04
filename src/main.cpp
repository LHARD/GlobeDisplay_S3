#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#include <secrets.h>

#define HOST_NAME "ESP32S3-GlobeDisplay"

#define DEBUG_ON 1
#define DEBUG_USE_SERIAL 0
#define DEBUG_USE_TELNET 1

#include "ESPTelnet.h"

ESPTelnet telnet;
AsyncWebServer server(8000);
AsyncWebSocket ws("/");


void loop() {
  sleep(1);
}




void wsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      telnet.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      telnet.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      telnet.print("wsData received");
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void loopCore0 (void* pvParameters) {
  const TickType_t msDelay = 100 / portTICK_PERIOD_MS;

  telnet.begin();
  ArduinoOTA.begin();
  
  ws.onEvent(wsEvent);
  server.addHandler(&ws);
  server.begin();

  while(1)
  {
    telnet.loop();
    ArduinoOTA.handle();
    ws.cleanupClients();
    vTaskDelay(msDelay);
  }
}

void setup() {
  Serial.begin(9600);
  sleep(5);
  Serial.println("S3 INIT...");
  
  WiFi.setHostname(HOST_NAME);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    sleep(5);
    ESP.restart();
  }

  xTaskCreatePinnedToCore (
    loopCore0,     // Function to implement the task
    "loopCore0",   // Name of the task
    10000,      // Stack size in bytes
    NULL,      // Task input parameter
    1,         // Priority of the task; 1 = LOW, 23 = HIGHEST
    NULL,      // Task handle.
    0          // Core where the task should run
  );

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}