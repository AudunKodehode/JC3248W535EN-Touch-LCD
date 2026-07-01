#include <JC3248W535EN-Touch-LCD.h>

//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder

//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I tested with the 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6

//Have to use screen.flush() for anything to change on the display.

JC3248W535EN screen;

uint16_t touchX, touchY;
void setup() {
  Serial.begin(115200);

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }
  //screen.clear(0, 0, 0); //makes screen black
  screen.clear(100, 100, 100);
  screen.setColor(255,255,255);
  screen.flush();
}

void loop() {
  if (screen.getTouchPoint(touchX, touchY)){
    Serial.println("Touch Pressed:" + String(touchX) + "," + String(touchY));
    screen.setColor(0,0,0);
    screen.drawFillRect(0, 0, 480, 20);
    screen.setColor(255,0,0);
    screen.prt((String(touchX) + "," + String(touchY)),0,0,2);
    screen.drawCircleOutline(touchX, touchY, 2);
	screen.flush();
  }
}
