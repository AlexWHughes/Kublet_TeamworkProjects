#include <Arduino.h>
#include <otaserver.h>
#include <kgfx.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "secrets.h"

OTAServer otaserver;
KGFX ui;
Preferences preferences;
StaticJsonDocument<2048> doc; // Adjust size based on expected JSON payload

const char *endpoint = "https://YOURURLHERE.teamwork.com/projects/api/v3/tasks.json";
const char *userID = "YOURUSERIDHERE"; // The userID for filtering tasks
unsigned long lastTime = 0;
unsigned long timerDelay = 600000;

TFT_eSprite textSpr = ui.createSprite(240, 150);

void setup() {
  Serial.begin(460800);
  Serial.println("Loading Teamwork Tasks");

  otaserver.connectWiFi(); // DO NOT EDIT
  otaserver.run();         // DO NOT EDIT

  ui.init();
  ui.clear();
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    otaserver.handle(); // DO NOT EDIT

    if (((millis() - lastTime) > timerDelay) || lastTime == (unsigned long)(0)) {
      HTTPClient http;
      http.begin(endpoint);
      http.addHeader("Authorization", String("Bearer ") + BEARER_TOKEN); // Use Bearer token for authentication
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

        JsonArray tasks = doc["tasks"];
        textSpr.fillSprite(TFT_BLACK); // Clear the sprite before drawing
        int y = 0;
        for (JsonObject task : tasks) {
          const char* assignee = task["assignee"]["id"];
          if (strcmp(assignee, userID) == 0) {
            const char* taskName = task["content"];
            ui.drawText(textSpr, taskName, Arial_24, TFT_YELLOW, 0, y);
            y += 30; // Adjust based on your font size
          }
        }
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
      lastTime = millis();
    }
  }

  delay(1);
}
