#include "src/JC3248W535EN-Touch-LCD.h"
#include "Roboto_Regular16pt7b.h"
#include "ShortBaby_Mg2w16pt7b.h"

//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder

//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I tested with the 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6

//Have to use screen.flush() for anything to change on the display.

// Custom fonts can be made using this tool: https://rop.nl/truetype2gfx/
JC3248W535EN screen;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }
  
  //screen.clear(0, 0, 0); //makes screen black
  screen.clear(100, 100, 100);
  screen.setColor(255, 255, 255);
  
  // Display text with supported characters
  screen.setFont(&Roboto_Regular16pt7b);
  screen.prt("Roboto", 50, 50, 1);
  screen.setFont(&ShortBaby_Mg2w16pt7b);
  screen.prt("Shortbaby", 50, 100, 1);
  
  screen.flush();
}

void loop() {
  // Nothing to do in the loop
}