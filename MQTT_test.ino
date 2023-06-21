#include <WiFi.h>
#include <PubSubClient.h>
#include "Arduino.h"
#include "heltec.h"
#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include <stdlib.h>

//Thermocouple board hardware spi pins
#define MAXCS   5
Adafruit_MAX31855 thermocouple(MAXCS);

// WiFi
const char* ssid = "MySpectrumWiFi90-2G"; // Enter your Wi-Fi name
const char* password = "bluecrown314"; // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "192.168.1.246";
const char *topic = "emqx/esp32";
//const char *mqtt_username = "emqx";
//const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//scale and offset
double scale = 1.0;
double offset= 0.0;

void setup() {
  //display setup
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->clear();
  Heltec.display->setContrast(255);
  delay(1000);
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    Serial.println(WiFi.localIP());
    String ipAddress = WiFi.localIP().toString();
    const char* ipCharArray = ipAddress.c_str();
    printBuffer(ipCharArray);
    delay(500);
  // wait for MAX chip to stabilize
  if (!thermocouple.begin()) {
    printBuffer("ERROR");
    while (1) delay(10);
  }
  printBuffer("DONE");

    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
   // client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.subscribe(topic);
}

/*void printBuffer(const char* str) {
  // Initialize the log buffer
 //allocate memory to store 8 lines of text and 30 chars per line.
Heltec.display->setLogBuffer(5, 30);
int length = strlen(str);
Heltec.display->setFont(ArialMT_Plain_16);

// Some test data

  for (uint8_t i = 0; i < length; i++) {
    Heltec.display->clear();
    // Print to the screen
    Heltec.display->print(str[i]);
    // Draw it to the internal screen buffer
    Heltec.display->drawLogBuffer(0, 0);
    // Display it on the screen
    Heltec.display->display();
  }

}*/

void printBuffer(const char* str) {
  // Initialize the log buffer
  // Allocate memory to store 5 lines of text and 30 chars per line.
  Heltec.display->setLogBuffer(5, 30);
  
  int length = strlen(str);
  Heltec.display->setFont(ArialMT_Plain_16);
  
  int16_t bufferWidth = length * 8; // Calculate the width of the text in the buffer
  int16_t x = (Heltec.display->getWidth() - bufferWidth) / 2;  // Calculate the x-position to center the text
  int16_t y = (Heltec.display->getHeight() - 16) / 2;  // Calculate the y-position to center the text

  Heltec.display->clear();
  
  // Print the entire string to the log buffer
  for (int i = 0; i < length; i++) {
    Heltec.display->print(str[i]);
  }
  
  // Draw the log buffer to the internal screen buffer, centered
  Heltec.display->drawLogBuffer(x, y);
  
  // Display it on the screen
  Heltec.display->display();
}

/*void callback(char *topic, byte *payload, unsigned int length) {
    Heltec.display->setLogBuffer(5, 30);
    int16_t bufferWidth = length * 8; // Calculate the width of the text in the buffer
    int16_t x = (Heltec.display->getWidth() - bufferWidth) / 2;  // Calculate the x-position to center the text
    int16_t y = (Heltec.display->getHeight() - 16) / 2;  // Calculate the y-position to center the text
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        Heltec.display->clear();
        // Print to the screen
        Heltec.display->print((char) payload[i]);
        // Draw it to the internal screen buffer
        Heltec.display->drawLogBuffer(x, y);
        // Display it on the screen
        Heltec.display->display();
    }

    delay(200);
    Serial.println();
    Serial.println("-----------------------");
    
}*/

void loop() {
    client.loop();
    char c_str[10];
    double c = thermocouple.readCelsius();
    if (isnan(c))
    {
      printBuffer("T/C Problem");
    }
    else
    {
    dtostrf(c, 4, 2, c_str);
    printBuffer(c_str);
    client.publish(topic, c_str);
   }
    delay(200);
}