# JC3248W535EN Touch LCD Library

A simple, lightweight library to drive the JC3248W535EN touch LCD display from Guition without requiring LVGL. This library provides an easy way to get started with your display using basic drawing functions and touch support.

Display: https://www.aliexpress.com/item/1005007566315926.html

## Overview

Many users struggle to get started with the JC3248W535EN display after purchasing it. This library serves as an excellent starting point, abstracting away the complex initialization and providing an intuitive API for common display operations.

## Features

- Easy display initialization
- Basic drawing primitives (rectangles, circles, triangles, lines, etc.)
- Text rendering
- Touch input handling
- QR code generation and display
- Portrait orientation support
- Example with command-line screen-design feature, featuring a python GUI designer.

## Dependencies

To use this library, you'll need to install the following before installing JC3248W535EN library:

1. [Arduino_GFX **(aka GFX Library for Arduino by moononournation)**](https://github.com/moononournation/Arduino_GFX) - For the display graphics
2. [JPEGDecoder by Bodmer](https://github.com/Bodmer/JPEGDecoder)
3. [Arduino esp32 board library by Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide)

## Installation

### Using Arduino IDE Library Manager (Recommended)

1. Open Arduino IDE
2. Go to Sketch -> Include Library -> Manage Libraries...
3. Search for "JC3248W535EN"
4. Click Install

### Manual Installation (IF Arduino Library Manager install above fails)

1. Download this repository as ZIP
2. In Arduino IDE, go to Sketch -> Include Library -> Add .ZIP Library...
3. Select the downloaded ZIP file
4. Restart Arduino IDE

### Arduino IDE Setup

For the JC3248W535EN board, use the <mark>**"ESP32S3 Dev Module"**</mark> board in Arduino.<br>
Set the following arduino settings for the ESP32S3 Dev Module up as follows under Tools menu: 

- USB CDC On Boot: "Enabled" // Important for Serial communication
- CPU Frequency: "240MHz (WiFi)"
- Core Debug Level: "None"
- USB DFU On Boot: "Disabled"
- Erase All Flash Before Sketch Upload: "Disabled"
- Events Run On: "Core 1"
- Flash Mode: "QIO 80MHz"
- Flash Size: "16MB (128Mb)"
- JTAG Adapter: "Disabled"
- Arduino Runs On: "Core 1"
- USB Firmware MSC On Boot: "Disabled"
- Partition Scheme: "8M with spiffs (3MB APP/1.5MB SPIFFS)" (Can also use Custom if using partitions.csv)
- PSRAM: "OPI PSRAM"
- Upload Mode: "UART0 / Hardware CDC"
- Upload Speed: "921600"
- USB Mode: "Hardware CDC and JTAG"
- Zigbee Mode: "Disabled"

**Note = If Using USB Keyboard and want your Serial device to work through the USB you need to use these 2 settings instead**
- Upload Mode: "USB-OTG CDC (TinyUSB)"
- USB Mode: "USB-OTG (TinyUSB)"

## Usage Examples

<mark>**NOTE: The TouchScreenAndSerialCommands example is prob the best example with the ButtonGuiClass usage**</mark><br>
Here are pics of the TouchScreenAndSerialCommands example with the 2 panels.

<img src="https://github.com/user-attachments/assets/8038cd5b-13cf-4ae0-8d81-6f2924f21115" alt="esp32_mainScreen" width="300">
<img src="https://github.com/user-attachments/assets/ad09598d-4818-4e99-8730-0f33f84e30c8" alt="esp32_panel1" width="300">
<img src="https://github.com/user-attachments/assets/5c8fdd44-ba0c-4ff7-83f2-4ffe8b4d9118" alt="esp32_panel2" width="300">

### Basic Initialization

```arduino
#include <JC3248W535EN-Touch-LCD.h>

JC3248W535EN screen;

void setup() {
  Serial.begin(115200);

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }
  
  // Clear the screen with white background
  screen.clear(255, 255, 255);
  screen.flush();  // HAVE TO USE THIS ANYTIME YOU WANT SCREEN TO UPDATE!
}

void loop() {
  // Your code here
}
```

### Drawing Shapes and Text

```arduino
#include <JC3248W535EN-Touch-LCD.h>

JC3248W535EN screen;

void setup() {
  Serial.begin(115200);
  screen.begin();
  screen.clear(255, 255, 255);
  
  // Draw a blue filled rectangle
  screen.setColor(0, 0, 255);
  screen.drawFillRect(50, 50, 100, 80);
  
  // Draw a red filled circle
  screen.setColor(255, 0, 0);
  screen.drawFillCircle(200, 150, 40);
  
  // Draw text
  screen.setColor(0, 0, 0);
  screen.prt("Hello World!", 120, 250, 2);

  screen.flush();  // HAVE TO USE THIS ANYTIME YOU WANT SCREEN TO UPDATE!
}

void loop() {
}
```

### Touch Input Example

```arduino
#include <JC3248W535EN-Touch-LCD.h>

JC3248W535EN screen;
uint16_t touchX, touchY;

void setup() {
  Serial.begin(115200);
  screen.begin();
  screen.clear(255, 255, 255);
  screen.setColor(0, 0, 0);
  screen.prt("Touch the screen!", 80, 200, 2);
  screen.flush();  // HAVE TO USE THIS ANYTIME YOU WANT SCREEN TO UPDATE!
}

void loop() {
  //check for touch interrupt. If no touch then immediatly return/continue
  if (!screen.screenWasTouched ){ continue; }
  else if (screen.getTouchPoint(touchX, touchY)) {
    // Draw a small circle where touch is detected
    screen.setColor(255, 0, 0);
    screen.drawFillCircle(touchX, touchY, 5);
    
    // Display coordinates
    screen.setColor(0, 0, 0);
    String coords = "X: " + String(touchX) + " Y: " + String(touchY);
    screen.clear(255, 255, 255);
    screen.prt(coords, 100, 240, 2);
    screen.flush();  // HAVE TO USE THIS ANYTIME YOU WANT SCREEN TO UPDATE!
    
    delay(100);  // Small delay to prevent too many readings
  }
}
```

### QR Code Generation

```arduino
#include <JC3248W535EN-Touch-LCD.h>

JC3248W535EN screen;

void setup() {
  Serial.begin(115200);
  screen.begin();
  screen.clear(255, 255, 255);
  
  // Draw a QR code (centered on screen)
  screen.drawQRCode("https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD", 
                   60, 120, 4, 255, 255, 255, 0, 0, 0);
                   
  screen.setColor(0, 0, 0);
  screen.prt("Scan me!", 120, 60, 2);
  screen.flush();  // HAVE TO USE THIS ANYTIME YOU WANT SCREEN TO UPDATE!
}

void loop() {
}
```

## Pin Configuration

This library uses the following default pin configuration for the JC3248W535EN display:

- Backlight: Pin 1
- Touch SDA: Pin 4
- Touch SCL: Pin 8
- Touch RST: Pin 12 ? GPIO 12 goes to the SD_MMC
- Touch INT: Pin 3  //This touch interrupt is not pin 11. Its GPIO 3

## Screen Orientation

The library handles the display in portrait orientation with proper coordinate mapping for ease of use.

## Contributing

Contributions to improve the library are welcome! Please feel free to submit pull requests.

## License

This project is released under the MIT License.
