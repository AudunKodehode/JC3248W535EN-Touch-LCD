#include <WiFi.h>
#include <HTTPClient.h>
#include <JC3248W535EN-Touch-LCD.h>
#include <SPIFFS.h>

//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder

//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I tested with the 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6

//Have to use screen.flush() for anything to change on the display.

JC3248W535EN screen;

// WiFi credentials
const char* ssid = "SSID";
const char* password = "Password";

void setup() {
  Serial.begin(115200);

  // Initialize SPIFFS first
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
  } else {
    Serial.println("SPIFFS initialized successfully");
  }

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }

  connectToWiFi();
  Serial.println("Loading image from the internet...");
  screen.loadImageFromUrl("https://raw.githubusercontent.com/AudunKodehode/JC3248W535EN-Touch-LCD/refs/heads/main/Examples/JpegFromWeb/480x320.jpg", 0, 0);
  screen.flush();
}

void connectToWiFi() {
  screen.prt("Connecting to WiFi...", 0, 0, 2);
  screen.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
  }
  screen.clear(0, 0, 0);
}

void loop() {
}
