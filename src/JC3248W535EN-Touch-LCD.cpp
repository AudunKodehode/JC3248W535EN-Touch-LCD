/**
 *  Made by AudunKodehode
 *
 *  See: https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD
 */

#include "JC3248W535EN_Touch_LCD.h"  // Note: Using the new header file name
#include <WiFi.h>        // Add WiFiClient support
#include <SPIFFS.h>      // Add SPIFFS support first, before JPEGDecoder
#include <HTTPClient.h>
#include "qrcode_helper.h" // https://github.com/ricmoo/QRCode
#include <JPEGDecoder.h>  // Include JPEGDecoder after SPIFFS to avoid conflicts

// Pin definitions
#define GFX_BL 1
#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
//#define TOUCH_RST_PIN 12		//Dont think this is 12 or exists. Pin 12 is the SD_MMC_CLK pin
#define TOUCH_PIN_NUM_INT 3			//Touch interrupt pin is I03/3 called TP_INT on guition schematic. This is not GPIO 11!
#define AXS_MAX_TOUCH_NUMBER 1

// You must define these static members in the .cpp file
volatile bool JC3248W535EN::screenWasTouched = false;
unsigned long JC3248W535EN::nextTouchScreenCheck = 0;

JC3248W535EN::JC3248W535EN() {
    bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
	g = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, width);
    gfx = new Arduino_Canvas(height, width, g, 0, 0, 0);
    currentR = currentG = currentB = 255;
    currentColor = 0xFFFF;
}

void IRAM_ATTR JC3248W535EN::touchISR_Lib(void) {
  if (millis() < nextTouchScreenCheck ){ return; }    //dont let an interrupt happen unless we have it armed. This is prob un-needed
  screenWasTouched = 1; //handle this in main .ino  and reset back to 0 when done
}

bool JC3248W535EN::begin() {
	//screenWasTouched = nextTouchScreenCheck = 0;
	
    // Initialize display
    if (!gfx->begin()) return false;
    
    // Initialize backlight
    if (GFX_BL != GFX_NOT_DEFINED) {
        pinMode(GFX_BL, OUTPUT);
        digitalWrite(GFX_BL, HIGH);
    }
    
    // Initialize touch i2c
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(TOUCH_I2C_CLOCK);
    
    // Configure touch pins
	/*
    pinMode(TOUCH_RST_PIN, OUTPUT);
    digitalWrite(TOUCH_RST_PIN, LOW);
    delay(200);
    digitalWrite(TOUCH_RST_PIN, HIGH);
    delay(200);
    */
	delay(400);	//do a dumb delay to match the previous delays
	
	//enable interrupt
	pinMode(TOUCH_PIN_NUM_INT, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(TOUCH_PIN_NUM_INT), touchISR_Lib, FALLING);
  
    gfx->fillScreen(0); // Clear screen
    return true;
}

uint16_t JC3248W535EN::mapX(uint16_t x, uint16_t y) {
    return height - y;
}

uint16_t JC3248W535EN::mapY(uint16_t x, uint16_t y) {
    return x;
}

uint16_t JC3248W535EN::rgb(uint8_t r, uint8_t g, uint8_t b) {
    return gfx->color565(r, g, b);
}

void JC3248W535EN::setColor(uint8_t r, uint8_t g, uint8_t b) {
    currentR = r;
    currentG = g;
    currentB = b;
    currentColor = rgb(r, g, b);
}

void JC3248W535EN::clear(uint8_t r, uint8_t g, uint8_t b) {
    gfx->fillScreen(rgb(r, g, b));
}
void JC3248W535EN::flush() {
    gfx->flush();
}

void JC3248W535EN::setFont(const GFXfont* font) {
    gfx->setFont(font);
}

// Drawing functions implementation
void JC3248W535EN::drawFillRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    int16_t px = height - (y + h);
    int16_t py = x;
    int16_t pw = h;
    int16_t ph = w;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px + pw > height) pw = height - px;
    if (py + ph > width) ph = width - py;
    gfx->fillRect(px, py, pw, ph, currentColor);
}
// Drawing functions implementation
void JC3248W535EN::drawFillRect2(int16_t x, int16_t y, int16_t w, int16_t h) {
    int16_t px = height - (y + h);
    int16_t py = x;
    int16_t pw = h;
    int16_t ph = w;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px + pw > height) pw = height - px;
    if (py + ph > width) ph = width - py;
    gfx->fillRect(px, py, pw, ph, currentColor);
}

void JC3248W535EN::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    // Convert coordinates to match the screen orientation
    int16_t px0 = height - y0;
    int16_t py0 = x0;
    int16_t px1 = height - y1;
    int16_t py1 = x1;
    
    // Draw the line with the current color
    gfx->drawLine(px0, py0, px1, py1, currentColor);
}

void JC3248W535EN::drawRect(int16_t x, int16_t y, int16_t w, int16_t h) {
    int16_t px = height - (y + h);
    int16_t py = x;
    int16_t pw = h;
    int16_t ph = w;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px + pw > height) pw = height - px;
    if (py + ph > width) ph = width - py;
    gfx->drawRect(px, py, pw, ph, currentColor);
}

void JC3248W535EN::drawFillCircle(int16_t x, int16_t y, int16_t radius) {
    int16_t px = height - y;
    int16_t py = x;
    gfx->fillCircle(px, py, radius, currentColor);
}

void JC3248W535EN::drawCircleOutline(int16_t x, int16_t y, int16_t radius) {
    int16_t px = height - y;
    int16_t py = x;
    gfx->drawCircle(px, py, radius, currentColor);
}

void JC3248W535EN::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    // Convert coordinates to match screen orientation
    int16_t px0 = height - y0;
    int16_t py0 = x0;
    int16_t px1 = height - y1;
    int16_t py1 = x1;
    int16_t px2 = height - y2;
    int16_t py2 = x2;
    
    gfx->drawTriangle(px0, py0, px1, py1, px2, py2, currentColor);
}

void JC3248W535EN::drawFillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    // Convert coordinates to match screen orientation
    int16_t px0 = height - y0;
    int16_t py0 = x0;
    int16_t px1 = height - y1;
    int16_t py1 = x1;
    int16_t px2 = height - y2;
    int16_t py2 = x2;
    
    gfx->fillTriangle(px0, py0, px1, py1, px2, py2, currentColor);
}

void JC3248W535EN::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius) {
    int16_t px = height - (y + h);
    int16_t py = x;
    int16_t pw = h;
    int16_t ph = w;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px + pw > height) pw = height - px;
    if (py + ph > width) ph = width - py;
    
    gfx->drawRoundRect(px, py, pw, ph, radius, currentColor);
}

void JC3248W535EN::drawFillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius) {
    int16_t px = height - (y + h);
    int16_t py = x;
    int16_t pw = h;
    int16_t ph = w;
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px + pw > height) pw = height - px;
    if (py + ph > width) ph = width - py;
    
    gfx->fillRoundRect(px, py, pw, ph, radius, currentColor);
}

void JC3248W535EN::drawEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry) {
    int16_t px = height - y;
    int16_t py = x;
    
    // Note: ellipses might need swapping rx and ry due to orientation change
    gfx->drawEllipse(px, py, ry, rx, currentColor);
}

void JC3248W535EN::drawFillEllipse(int16_t x, int16_t y, int16_t rx, int16_t ry) {
    int16_t px = height - y;
    int16_t py = x;
    
    // Note: ellipses might need swapping rx and ry due to orientation change
    gfx->fillEllipse(px, py, ry, rx, currentColor);
}

void JC3248W535EN::prt(const String &text, int x, int y, uint8_t size) {
    uint8_t originalRotation = gfx->getRotation();
    gfx->setRotation(1);
    gfx->setTextColor(currentColor);
    gfx->setTextSize(size);
    gfx->setCursor(x, y);
    gfx->println(text);
    gfx->setRotation(originalRotation);
}

void JC3248W535EN::clearTouchData(void){
	//there is prob a much more elegant way but for now just read the i2c data
	getTouchPoint(xT, yT);
}

//this may be compatible with the AXS15231B touchscreen?
bool JC3248W535EN::getTouchPoint(uint16_t &x, uint16_t &y) {
    uint8_t data[AXS_MAX_TOUCH_NUMBER * 6 + 2] = {0};
    
    // Define the read command array properly
    const uint8_t read_cmd[11] = {
        0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
        (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) >> 8),
        (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) & 0xff),
        0x00, 0x00, 0x00
    };
    
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(read_cmd, 11);
    if (Wire.endTransmission() != 0) return false;
    delay(1);	//add a tiny delay before the read?
	
    if (Wire.requestFrom(TOUCH_ADDR, sizeof(data)) != sizeof(data)) return false;
    
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = Wire.read();
    }
    
    if (data[1] > 0 && data[1] <= AXS_MAX_TOUCH_NUMBER) {
        uint16_t rawX = ((data[2] & 0x0F) << 8) | data[3];
        uint16_t rawY = ((data[4] & 0x0F) << 8) | data[5];
        
        if (rawX == 273 && rawY == 273) return false;
        if (rawX > 4000 || rawY > 4000) return false;
        
        yT = y = map(rawX, 0, height, height, 0);	//appears to transpose the coordinate map so the top left corner is 0,0
        xT = x = rawY;
        
		//add dumb fudge factor for since the top left corner is 14,10 not 0,0 and bottom right is 461,308 not 480,320
		#ifdef USE_TOUCHSCREEN_FUDGE_FACTOR
			y = map(y, 10, 308, 0, height);	//try to fudge factor fix
			x = map(x, 14, 461, 0, width);	//try to fudge factor fix
		#endif
		
		//prob want to add a range check here to ensure the click is truly in bounds
		if (x > width || y > height ){
			return false;
		}
		else{
			return true;
		}
    }
    
    return false;
}

void JC3248W535EN::drawQRCode(const char* data, uint16_t x, uint16_t y, uint8_t moduleSize,
                              uint8_t bgColorR, uint8_t bgColorG, uint8_t bgColorB,
                              uint8_t fgColorR, uint8_t fgColorG, uint8_t fgColorB) {
   // Create QR code
   QRCode qrcode;
   
   // Adjust version based on data length to ensure it fits
   // A higher version can store more data but creates a larger QR code
   uint8_t version = 3;  // Default for ~65 alphanumeric characters
   
   // Increase version for longer data
   size_t dataLen = strlen(data);
   if (dataLen > 65 && dataLen <= 114) {
     version = 4;  // Up to ~114 alphanumeric characters
   } else if (dataLen > 114 && dataLen <= 154) {
     version = 5;  // Up to ~154 alphanumeric characters
   } else if (dataLen > 154 && dataLen <= 195) {
     version = 6;  // Up to ~195 alphanumeric characters
   } else if (dataLen > 195) {
     version = 8;  // Up to ~310 alphanumeric characters
     // If your data is longer than 310 characters, you may need to increase version further
   }
   
   // Calculate buffer size and allocate memory
   uint16_t bufferSize = qrcode_getBufferSize(version);
   uint8_t* qrcodeData = new uint8_t[bufferSize];
   
   if (qrcodeData == NULL) {
     // Memory allocation failed
     Serial.println("Failed to allocate memory for QR code");
     return;
   }
   
   // Initialize the QR code
   qrcode_initText(&qrcode, qrcodeData, version, 0, data);
   
   Serial.print("QR code version ");
   Serial.print(version);
   Serial.print(", size: ");
   Serial.print(qrcode.size);
   Serial.println("x" + String(qrcode.size));
   
   // Set background color and draw background
   setColor(bgColorR, bgColorG, bgColorB);
   drawFillRect(x, y, qrcode.size * moduleSize, qrcode.size * moduleSize);
   
   // Set QR code color
   setColor(fgColorR, fgColorG, fgColorB);
   
   // Draw each dark module (skip drawing light modules since they're the same as background)
   for (uint8_t qrY = 0; qrY < qrcode.size; qrY++) {
     for (uint8_t qrX = 0; qrX < qrcode.size; qrX++) {
       // Only draw the dark modules (more efficient)
       if (qrcode_getModule(&qrcode, qrX, qrY)) {
         drawFillRect2(
           x + qrX * moduleSize,
           y + qrY * moduleSize,
           moduleSize,
           moduleSize
         );
       }
     }
   }
   
   // Add a flush at the end for efficiency
   flush();
   
   // Free allocated memory
   delete[] qrcodeData;
}

void JC3248W535EN::image(const uint16_t* bitmap, int16_t x, int16_t y, int16_t w, int16_t h) {
    // Transform coordinates to match screen orientation
    // The screen is rotated 90 degrees, so we need to convert from normal coordinates
    int16_t px = height - (y + h);  // Start at the top-left after transformation
    int16_t py = x;

    // Check bounds and clip if necessary
    if (px < 0) {
        px = 0;
    }
    if (py < 0) {
        py = 0;
    }
    
    int16_t pw = h;  // Width and height are swapped due to orientation
    int16_t ph = w;
    
    if (px + pw > height) {
        pw = height - px;
    }
    if (py + ph > width) {
        ph = width - py;
    }
    
    // We need to render each pixel individually as drawBitmap only accepts uint8_t* and our bitmap is uint16_t*
    for (int16_t i = 0; i < ph; i++) {
        for (int16_t j = 0; j < pw; j++) {
            uint16_t color = bitmap[i * w + j]; // Get color from bitmap
            gfx->drawPixel(px + j, py + i, color);
        }
    }
    
    // We need to flush to update the display
    flush();
}

// Modified getPixel function that just returns a default value without serial logging
uint16_t JC3248W535EN::getPixel(int16_t x, int16_t y) {
    // Transform coordinates to match screen orientation
    int16_t px = height - y;
    int16_t py = x;
    
    // Check bounds
    if (px < 0 || px >= height || py < 0 || py >= width) {
        return 0; // Return black for out-of-bounds
    }
    
    // Arduino_Canvas doesn't support reading pixel color directly
    // Return a default value - we simply return white as a default
    return 0xFFFF;
}

void JC3248W535EN::fetchJpeg(const char* url, int16_t x, int16_t y) {
    // Check if SPIFFS is initialized
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    HTTPClient http;
    
    Serial.print("Downloading JPEG from: ");
    Serial.println(url);
    
    // Start HTTP connection with better timeout handling
    http.begin(url);
    http.setTimeout(15000); // 15 second timeout for slow connections
    
    // Get the HTTP response code
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        Serial.print("HTTP GET failed, error code: ");
        Serial.println(httpCode);
        http.end();
        
        // Draw error indicator
        setColor(200, 50, 50);
        drawFillRect(x, y, 80, 20);
        setColor(255, 255, 255);
        prt("HTTP " + String(httpCode), x + 5, y + 15, 1);
        return;
    }
    
    // Get the downloaded data
    WiFiClient* stream = http.getStreamPtr();
    
    // Create a temporary file to store the image
    // ESP32 has SPIFFS file system we can use
    File jpgFile = SPIFFS.open("/temp.jpg", FILE_WRITE);
    if (!jpgFile) {
        Serial.println("Failed to create temporary file");
        http.end();
        return;
    }
    
    // Read all data from the HTTP stream and write it to the file
    size_t totalBytes = http.getSize();
    size_t bytesRead = 0;
    uint8_t buff[512] = {0};
    
    Serial.print("Image size: ");
    Serial.print(totalBytes);
    Serial.println(" bytes");
    
    if (totalBytes <= 0) {
        Serial.println("Invalid content length");
        jpgFile.close();
        http.end();
        return;
    }
    
    // Draw a loading indicator
    setColor(50, 50, 200);
    drawRect(x, y, 100, 20);
    
    // Download the file
    unsigned long startTime = millis();
    while (http.connected() && (bytesRead < totalBytes) && (millis() - startTime < 15000)) {
        size_t available = stream->available();
        if (available) {
            size_t readBytes = stream->readBytes(buff, (available < sizeof(buff)) ? available : sizeof(buff));
            if (readBytes > 0) {
                jpgFile.write(buff, readBytes);
                bytesRead += readBytes;
                
                // Optional: Show download progress
                if (totalBytes > 0) {
                    Serial.print("Download progress: ");
                    Serial.print((bytesRead * 100) / totalBytes);
                    Serial.println("%");
                    
                    // Update progress bar
                    int progressWidth = (bytesRead * 98) / totalBytes;
                    drawFillRect(x+1, y+1, progressWidth, 18);
                }
            }
        } else {
            delay(1); // Small delay to give WiFi stack some time
        }
    }
    
    // Clear progress bar
    setColor(0, 0, 0);
    drawFillRect(x, y, 100, 20);
    
    jpgFile.close();
    http.end();
    
    if (bytesRead < totalBytes) {
        Serial.println("Download incomplete or timed out");
        return;
    }
    
    // Now open the file for reading and decode it
    jpgFile = SPIFFS.open("/temp.jpg", FILE_READ);
    if (!jpgFile) {
        Serial.println("Failed to open temporary file for reading");
        return;
    }
    
    // Get the file size
    size_t fileSize = jpgFile.size();
    Serial.print("File size: ");
    Serial.println(fileSize);
    
    if (fileSize <= 0) {
        Serial.println("Empty file");
        jpgFile.close();
        return;
    }
    
    // Read file into buffer for JPEGDecoder
    uint8_t* jpegBuffer = new uint8_t[fileSize];
    
    if (!jpegBuffer) {
        Serial.println("Failed to allocate memory for JPEG buffer");
        jpgFile.close();
        
        // Draw error indicator - memory issue
        setColor(200, 100, 50);
        drawFillRect(x, y, 80, 20);
        setColor(255, 255, 255);
        prt("Memory err", x + 5, y + 15, 1);
        return;
    }
    
    // Read the file into the buffer
    size_t bytesReadFromFile = jpgFile.read(jpegBuffer, fileSize);
    jpgFile.close();
    
    if (bytesReadFromFile != fileSize) {
        Serial.println("Failed to read complete file");
        delete[] jpegBuffer;
        return;
    }
    
    // Start the decoder
    Serial.println("Starting JPEG decoder...");
    if (!JpegDec.decodeArray(jpegBuffer, fileSize)) {
        Serial.println("JPEG decode failed");
        delete[] jpegBuffer;
        
        // Draw a fallback image/icon when JPEG decoding fails
        setColor(70, 70, 100);
        drawFillRect(x, y, 100, 80);
        setColor(255, 255, 255);
        prt("JPG Format", x + 10, y + 30, 1);
        prt("Not Supported", x + 5, y + 50, 1);
        return;
    }
    
    delete[] jpegBuffer;
    
    // Get the image information
    uint16_t jpegWidth = JpegDec.width;
    uint16_t jpegHeight = JpegDec.height;
    
    Serial.print("JPEG dimensions: ");
    Serial.print(jpegWidth);
    Serial.print("x");
    Serial.println(jpegHeight);
    
    // Render the image
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;
    
    // Render the image by reading blocks (MCUs)
    while (JpegDec.read()) {
        // Get the coordinates of the MCU (Minimum Coded Unit)
        uint16_t mcuX = JpegDec.MCUx * mcu_w;
        uint16_t mcuY = JpegDec.MCUy * mcu_h;
        
        // Get the decoded MCU data
        uint16_t* pImg = JpegDec.pImage;
        
        // Calculate how many pixels to draw for this MCU (may be clipped at image boundaries)
        // Fix min function by explicitly casting to uint16_t to match types
        uint16_t mcuWidth = (uint16_t)min((uint32_t)mcu_w, max_x - mcuX);
        uint16_t mcuHeight = (uint16_t)min((uint32_t)mcu_h, max_y - mcuY);
        
        // Draw each pixel in the MCU
        for (uint16_t py = 0; py < mcuHeight; py++) {
            for (uint16_t px = 0; px < mcuWidth; px++) {
                // Get the pixel color
                uint16_t pixelColor = *pImg++;
                
                // Calculate the position where to draw the pixel
                int16_t drawX = x + mcuX + px;
                int16_t drawY = y + mcuY + py;
                
                // Transform coordinates to match screen orientation
                int16_t transformedX = height - drawY;
                int16_t transformedY = drawX;
                
                // Draw the pixel if it's within screen bounds
                if (transformedX >= 0 && transformedX < height && 
                    transformedY >= 0 && transformedY < width) {
                    // Draw the pixel using the GFX library
                    gfx->drawPixel(transformedX, transformedY, pixelColor);
                }
            }
        }
    }
    
    // Update the display when done
    flush();
    
    // Clean up the decoder
    JpegDec.abort();
    
    Serial.println("JPEG rendering completed");
}

bool JC3248W535EN::loadImageFromUrl(const char* url, int16_t x, int16_t y) {
  Serial.printf("Loading image: %s\n", url);
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return false;
  }
  
  // Show loading indicator
  setColor(30, 30, 30);
  drawFillRoundRect(x, y, 100, 80, 5);
  setColor(200, 200, 200);
  prt("Loading...", x + 10, y + 40, 1);
  flush();
  
  // Initialize SPIFFS if needed
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS init failed");
    return false;
  }
  
  // Remove any existing temporary file
  if (SPIFFS.exists("/temp.img")) SPIFFS.remove("/temp.img");
  
  // Download the image
  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }
  
  // Get image info and create progress bar
  int contentLength = http.getSize();
  String contentType = http.header("Content-Type");
  Serial.printf("Content-Type: %s, Size: %d bytes\n", contentType.c_str(), contentLength);
  
  setColor(70, 70, 70);
  drawFillRect(x + 5, y + 60, 90, 10);
  
  // Save to file
  File imgFile = SPIFFS.open("/temp.img", FILE_WRITE);
  if (!imgFile) {
    Serial.println("Failed to create file");
    http.end();
    return false;
  }
  
  // Download data with progress bar
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buffer[1024];
  size_t totalRead = 0;
  
  while (http.connected() && (totalRead < contentLength)) {
    size_t available = stream->available();
    if (available) {
      // Change this line to use a proper size comparison
      size_t bytesRead = stream->readBytes(buffer, available < sizeof(buffer) ? available : sizeof(buffer));
      if (bytesRead > 0) {
        imgFile.write(buffer, bytesRead);
        totalRead += bytesRead;
        
        // Update progress bar
        int progress = (totalRead * 100) / contentLength;
        int barWidth = (progress * 90) / 100;
        setColor(0, 200, 100);
        drawFillRect(x + 5, y + 60, barWidth, 10);
        
        if (progress % 20 == 0) {
          Serial.printf("Download: %d%%\n", progress);
        }
      }
    } else {
      delay(1);
    }
  }
  
  imgFile.close();
  http.end();
  
  // Clear progress bar area
  setColor(0, 0, 0);
  drawFillRect(x, y, 100, 80);
  
  // Now decode and render the image - JPEG only for simplicity
  imgFile = SPIFFS.open("/temp.img", FILE_READ);
  if (!imgFile) return false;
  
  size_t fileSize = imgFile.size();
  if (fileSize == 0) {
    imgFile.close();
    return false;
  }
  
  // Read file into memory
  uint8_t* imgBuffer = new uint8_t[fileSize];
  if (!imgBuffer) {
    imgFile.close();
    return false;
  }
  
  imgFile.read(imgBuffer, fileSize);
  imgFile.close();
  
  // Decode and render the JPEG
  Serial.println("Decoding image...");
  if (!JpegDec.decodeArray(imgBuffer, fileSize)) {
    delete[] imgBuffer;
    Serial.println("Decode failed");
    return false;
  }
  
  delete[] imgBuffer;
  
  // Get image dimensions
  uint16_t w = JpegDec.width;
  uint16_t h = JpegDec.height;
  Serial.printf("Image size: %dx%d\n", w, h);
  
  // Render the image using JPEGDecoder's MCU approach
  uint32_t mcuWidth = JpegDec.MCUWidth;
  uint32_t mcuHeight = JpegDec.MCUHeight;
  
  while (JpegDec.read()) {
    uint16_t mcuX = JpegDec.MCUx * mcuWidth;
    uint16_t mcuY = JpegDec.MCUy * mcuHeight;
    uint16_t* pImg = JpegDec.pImage;
    
    // Fix the type mismatch in these two lines by explicit casting
    uint16_t mcu_w = (uint16_t)((mcuWidth < (uint32_t)(w - mcuX)) ? mcuWidth : (uint32_t)(w - mcuX));
    uint16_t mcu_h = (uint16_t)((mcuHeight < (uint32_t)(h - mcuY)) ? mcuHeight : (uint32_t)(h - mcuY));
    
    // Draw each pixel
    for (uint16_t py = 0; py < mcu_h; py++) {
      for (uint16_t px = 0; px < mcu_w; px++) {
        uint16_t color = *pImg++;
        int16_t drawX = x + mcuX + px;
        int16_t drawY = y + mcuY + py;
        
        // Apply coordinate transformation for rotated display
        int16_t tx = height - drawY;
        int16_t ty = drawX;
        
        if (tx >= 0 && tx < height && ty >= 0 && ty < width) {
          gfx->drawPixel(tx, ty, color);
        }
      }
    }
  }
  
  // Clean up
  JpegDec.abort();
  flush();
  Serial.println("Image rendered successfully");
  return true;
}