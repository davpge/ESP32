/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ArduinoJson.h>  // Include the ArduinoJSON library

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x04, 0x67, 0xB1};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2

// JSON buffer size (adjust according to your data size)
const int JSON_BUFFER_SIZE = 256;

// Create a JSON buffer
StaticJsonDocument<JSON_BUFFER_SIZE> jsonBuffer;

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Create JSON object
    JsonObject root = jsonBuffer.to<JsonObject>();

    // Set values to send in JSON object
    root["id"] = BOARD_ID;
    root["x"] = random(1, 50);
    root["y"] = random(1, 50);

    // Serialize JSON to a string
    String jsonData;
    serializeJson(root, jsonData);

    // Send message via ESP-NOW
    esp_now_send(0, (uint8_t*)jsonData.c_str(), jsonData.length() + 1); // Include the null terminator

    lastTime = millis();
  }
}