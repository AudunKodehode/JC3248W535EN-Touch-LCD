////////////////////////////////////////////////////////////////////////////////////////
//EDIT userSpecificSetup.h for your wifi and ftp info
//Verified working on Arduino IDE 2.3.8 with esp32 Arduino Release v3.3.7 based on ESP-IDF v5.5.2 as well as  v3.3.8 based on ESP-IDF v5.5.4
//Created a custom partitions.csv that gives 2 4.5MB partitions and a 6.5MB SPIFFS partition. May need to 1st erase flash to get this to work
//
//Hardware is Guition JC3248W535EN / JC3248W535 /JC3248W535C
//schematics  here https://github.com/clowrey/ESPhome-JC3248W535EN?tab=readme-ov-file
//https://www.letscontrolit.com/forum/viewtopic.php?t=10480
//
//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder
//Arduino SimpleFTPServer by Renzo. Verified working with v3.0.2
//
//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I started with this 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6
//My updates are here
//https://github.com/beachmiles/JC3248W535EN-Touch-LCD
//Have to use screen.flush() for anything to change on the display.
//
// If you want LVGL this works https://github.com/NorthernMan54/JC3248W535EN
// Other examples I havent tested https://community.home-assistant.io/t/guition-3-5-320x480-jc3248w535en-lcd-dev-board-working-example/774022
// https://spotpear.com/forum-issue/ESP32-S3-3.5-inch-LCD-Captive-TouchScreen-Display-480x320-Tablet-MP3-Video-Weather-Clock.html
// https://f1atb.fr/home-automation/esp32-s3/esp32-s3-3-5-inch-capacitive-touch-ips-display-setup/

//wifi stuff
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
#include <esp_wifi.h>
#include "time.h"

#include "userSpecificSetup.h"   //update this to use your wifi and ftp passwords

//touchscreen stuff. If battery is connected and USB at the same time I was getting some erratic touches but otherwise USING_TOUCH_INT 1 is solid1
#define USING_TOUCH_INT 1         //lets us know we are using the touch interrupt function instead of polling the i2c bus
#include <JC3248W535EN-Touch-LCD.h>
#include "ButtonGuiClass.h"     //uses the JC3248W535EN-Touch-LCD functions

//Add SD librarys. Found pincfg.j in DEMO_MJPEG folder
#include "SD_MMC.h"
#include "FS.h"
#include "pincfg.h"     //our custom pins on the Guition board
//#include "ff.h"       //try to sync time with SD_MMC
/////////////// END SD CARD STUFF

//you can define these in userSpecificSetup.h if you want to use them
#ifdef USE_FTP_SERVER
  //https://github.com/greiman/SdFat
  //#include <SdFat.h>

  //https://github.com/xreef/SimpleFTPServer
  //https://mischianti.org/how-to-build-an-arduino-ftp-server-uno-mega-ethernet-sd-simpleftpserver-v3-x/
  #include "FtpServerKey.h"               //Working with 3.0.2 version import our custom esp32 defaults to use SD_MMC, etc. May want to delete the build folder here if C:\Users\<user>\AppData\Local\arduino\sketches if this doesnt take
  #include <SimpleFTPServer.h>            //SimpleFTPServer.h. Working with 3.0.2 version. But file date times do not work
  //#include "SimpleFTPServer_SD_MMC.h"   //SimpleFTPServer_SD_MMC.h I forced to be in SD_MMC mode
  FtpServer ftpSrv;                       //the ftp client is not showing the right date time but the SD Card has the right time
#endif

//COMPILE FLAGS FROM GOOD ARDUINO SETTINGS
/*
CPU=240000000L -DARDUINO=10607 -DARDUINO_ESP32S3_DEV -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD="ESP32S3_DEV" -DARDUINO_VARIANT="esp32s3" -DARDUINO_PARTITION_default_8MB 
-DARDUINO_HOST_OS="windows" -DARDUINO_FQBN="esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=cdc,CPUFreq=240,FlashMode=qio,FlashSize=16M,PartitionScheme=default_8MB,DebugLevel=none,PSRAM=opi,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default,ZigbeeMode=default" 
-DESP32=ESP32 -DCORE_DEBUG_LEVEL=0 -DARDUINO_RUNNING_CORE=1 -DARDUINO_EVENT_RUNNING_CORE=1 -DBOARD_HAS_PSRAM -DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1 -DARDUINO_USB_MSC_ON_BOOT=0 -DARDUINO_USB_DFU_ON_BOOT=0 
*/

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
  #error "Wrong board selected! This code is designed for ESP32-S3 only."
#endif

//ARDUINO_PARTITION_default_16MB and ARDUINO_NANO_ESP32 DNE
#if defined(ARDUINO_PARTITION_default_8MB)
  //THIS IS A GOOD SETTING
  //#pragma message("Correct partition 8MB Flash (3MB APP/1.5MB SPIFFS) detected")

//This is the custom partition scheme
#elif defined(ARDUINO_PARTITION_)
  //THIS IS A GOOD SETTING
  //#pragma message("Using custom 16MB partiton")
#else
  #warning "Please select Custom or 8MB Flash (3MB APP/1.5MB SPIFFS) in Arduino Settings and Flash Size 16MB"
#endif

#if !defined(BOARD_HAS_PSRAM)
  #error "PSRAM is not enabled! Enable OPI PSRAM Tools > PSRAM."
  //#warning "PSRAM is not enabled! Enable OPI PSRAM Tools > PSRAM."
#elif !defined(CONFIG_SPIRAM_MODE_OCT)
  #error "Wrong PSRAM mode! Please select 'OPI PSRAM' in the Tools menu."
#else
  //#pragma message "Correct OPI (Octal) PSRAM enabled."
#endif

#if ARDUINO_USB_CDC_ON_BOOT == 0
  #error "Please enable 'USB CDC On Boot' in the Tools menu before compiling."
#endif

//if we are using the TinyUsb library we appear to have to init this device even if we dont use it to be able to have the reboot to bootloader function work?
#ifdef USE_HID_KEYBOARD
  //Add keyboard libraries. 
  //This kills the Serial port unless you enable Upload Mode=USB-OTG CDC (TinyUSB) and USB MODE=USB-OTG (TinyUSB) in arduino settings
  #include "USB.h"
  #include "USBHIDKeyboard.h"
  USBHIDKeyboard Keyboard;

  #if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 0)
    //#pragma message "USE_HID_KEYBOARD and USB Mode: USB-OTG (TinyUSB) is selected."
  #else
    #error "USE_HID_KEYBOARD set and TinyUSB is Needed if you want to use HID keyboard/mouse and also have normal Serial ouput out the USB CDC port"
  #endif
#else
  //even if we are not using the keyboard if have TinyUsb enabled we need to intantiate this hardware
  #if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 0)
    #include "USB.h"
    #include "USBHIDKeyboard.h"
    USBHIDKeyboard Keyboard;
  #endif
#endif

#include "esp_system.h"
#include "soc/rtc_cntl_reg.h"    //has esp32 register for reboot to bootloader function. This is the older way evidently. Used to work

JC3248W535EN screen;                //The main display variable
JC3248W535EN *screenPtr = &screen;  //Pointer used for button functions
//Arduino_GFX *gfx = screen.gfx;      //in case anyone want to use direct gfx calls. Will not work seamlessly with "JC3248W535EN screen" as the rotation is different

static const String programVersion = "1.0.6";    //program version
uint16_t touchX, touchY;            //touch screen variables.
String curTestPanel = "Test";

//vars for clock
static uint8_t hh, mm, ss;
static unsigned long targetTime = 0; // next action time for clock
unsigned long cur_millis;
String clockString;                  // holds the created time screen. Happens once a second

uint8_t systemInitState = 0;
bool wifiStarted = false;
bool ftpStarted = false;
bool ntpSynced = false;
bool otaSetup = false;
bool screenWorking = false;
unsigned long last_ota_time = 0; 
volatile bool isOtaHappening = 0;
unsigned long nextNetworkConfigCheck = 0;
uint8_t cardType = CARD_NONE;       //SD card type
int sdBootCounter = 1;              //power cycle counter that we read the file off the SD card to.
float batteryVoltage = 0;
float batteryVoltageAtStartup = 0;

//The datasheet says resistor divider R26=33K and R27=100K and R25=0ohms going to GPIO5. 
//Taking a pic of board showed R26=683 and R27=01D so R26=68K and R27=100K
//I measured R26=58k R27=80.5K R25=2.31V with a 3.91V being read at R26 w battery attached. (These resistor reads were not solid and constantly rising for some reason!)
//const float dividerRatio = 1.33; // Divider Ratio: (33k + 100k) / 100k = 1.33 . 
//const float dividerRatio = 1.72; // Actual measured: (58k + 80.5k) / 80.5k
const float dividerRatio = 1.68; // Actual resistors: R26=68k, R27=100k

//Test and Panel 1 buttons
ButtonGuiClass LastTouch    (screenPtr, 325, 40, 150, 28, 255, 100, 100, "Last Touch", 2);   //PANEL 1 button showing the last touch time
ButtonGuiClass CS           (screenPtr, 325, 70, 150, 28, 200, 255, 255, "Clear Screen", 2); //PANEL 1 button to clear the screen
ButtonGuiClass ColorRand    (screenPtr, 325, 100, 150, 28, 100, 255, 255, "Random Color", 2);  //PANEL 1 button to make a random background color
ButtonGuiClass ListSdFILES  (screenPtr, 325, 130, 150, 28, 255, 255, 255, "List Files", 2);  //PANEL 1 button to make a random background color
ButtonGuiClass TestModeBut  (screenPtr, 325, 160, 150, 28, 255, 255, 255, "TestMode Off", 2);  //PANEL 1 button to make a random background color

//Panel 2 buttons
ButtonGuiClass ResetChip    (screenPtr, 325, 40, 150, 28, 0xFF, 0xFF, 100, "Reset Esp32", 2);   //PANEL 2 button to reset esp32
ButtonGuiClass ResetToBL    (screenPtr, 325, 70, 150, 28, 0xFF, 100, 100, "Enter BootLDR", 2);   //PANEL 2 button to reset esp32

ButtonGuiClass Panel1Button (screenPtr, 0, 300, 230, 28, 200, 255, 255, "Switch to Panel 1", 2);     //button to switch to panel 1
ButtonGuiClass Panel2Button (screenPtr, 240, 300, 230, 28, 255, 255, 100, "Admin Settings", 2);   //button to switch to panel 2

void setup() {
  initAndCheckEspHw();
  initScreen();
  printDebugInfo();
  initKeyboard();
  initWifi(0);
  syncTimeToNTP(0);
  initArduinoOTA(0);
  initSDcard();  //init SD card after connecting to wifi and syncing time with NTP   
  incrementBootCounterOnSdCard();
  initFtpServer();
}

void loop() {
  processSerialCommand();
  handleTouchScreen();
  displayUptime();
  handleNetworkTasks();
}

void initAndCheckEspHw(){
  WRITE_PERI_REG(RTC_CNTL_OPTION1_REG, 0);  //ensure we do not stay in the bootloader?

  Serial.begin(115200);   // 1. Native USB Serial (CDC)
  //while (!Serial) {delay(10); } //wait for serial monitor to open

  #ifdef USE_SERIAL0
  //The default Serial0 setup should work as it should use the right pins 
  //Serial0.begin(115200, SERIAL_8N1, HeaderP1_4PIN_PIN3_RXD2, HeaderP1_4PIN_PIN2_TXD2); //Guition Hardware UART. This should go to the physical TX/RX pins out of port 1
  Serial0.begin(115200);  //
  #endif

  delay(800);             //give some time for terminal app to re-connect

  printResetReason();

  //ESP.getFlashChipSize() returns the size you selected in the IDE, not necessarily the physical hardware capacity
  //Serial.printf("IDE Configured Flash: %u bytes\n", ESP.getFlashChipSize());
  Serial.println("IDE Configured Flash Size=" + String(ESP.getFlashChipSize() / 1024 / 1024) + "MB");

  if (psramFound()) {
    //Serial.printf("PSRAM available! Size: %d bytes\r\n", ESP.getPsramSize());
    Serial.println("PSRAM available! Size=" + String(ESP.getPsramSize() / 1024 / 1024) + "MB");
  } else {
    Serial.println("PSRAM not detected from psramFound function. Change PSRAM to OPI mode to enable");
  }

  initADC();
}

void initADC(){
  //Init ADC for "BAT_ADC_PIN" on GPIO 5
  pinMode(BAT_ADC_PIN, INPUT);  // explicitly set input, no pullup. This may be needed

  //Attenuation: To measure 3100mV, use ADC_ATTEN_DB_11 or ADC_ATTEN_DB_12. This acts like an internal voltage divider.
  //analogSetAttenuation(ADC_ATTENDB_MAX); //Sets the attenuation globally for all ADC pins. To measure higher voltages (up to ~3.1V), the range is typically limited to ~1.1V. 
  analogSetPinAttenuation(BAT_ADC_PIN, ADC_ATTENDB_MAX);  //may not need this as well? ADC_ATTENDB_MAX or ADC_ATTEN_DB_11

  //Unlike the original ESP32, the ESP32-S3 includes analogReadMilliVolts(), which uses factory-burned eFuse calibration values to provide accurate voltage readings (0–3100 mV)
  //ADC Default for esp32-S3 is 12bits, but you can manually set it (9-13 bits) but will need to do oversampling to get true 13 bits of data
  analogReadResolution(12);   //Sets range to 0-4095
  //analogReadResolution(13); //Sets range to 0-8191
  delay(1);                   //do a dummy delay

  readBatteryVoltage(1);
  batteryVoltageAtStartup = batteryVoltage; //save initial battery voltage reading
}

//NOTE IF USB is connected with power this voltage will be the charger output ~4.2V. 
//I am reading ~4.13V with my setup with USB connected. Only when running on battery power alone will you see the actual battery voltage.
//The datasheet says resistor divider R26=33K and R27=100K and R25=0ohms going to GPIO5. 
//Taking a pic of board showed R26=683 and R27=01D so R26=68K and R27=100K
//I measured R26=58k R27=80.5K R25=2.31V with a 3.91V being read at R26 w battery attached. (These resistor reads were not solid and constantly rising for some reason!)
void readBatteryVoltage(bool printOutput){
  //Read the raw ADC value (0-4095)
  //int rawValue = analogRead(BAT_ADC_PIN);
  //float batteryVoltage3 = (rawValue / 4095.0) * 3.1 * dividerRatio;

  // Read calibrated millivolts at the pin (0 - 3158mV)
  // This automatically uses internal eFuse calibration for better accuracy
  uint32_t pinMV = analogReadMilliVolts(BAT_ADC_PIN);

  // Convert back to the source voltage (0 - 4200mV)
  batteryVoltage = (pinMV * dividerRatio) / 1000.0; 

  if (printOutput){
    Serial.printf("Battery voltage RawPin=%u mV | Adjusted Voltage=%.3f V\r\n", pinMV, batteryVoltage);
    #ifdef USE_SERIAL0
      Serial0.printf("Battery voltage RawPin=%u mV | Adjusted Voltage=%.3f V\r\n", pinMV, batteryVoltage);
    #endif
  }
}

void initScreen(){
  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    #ifdef USE_SERIAL0
      Serial0.println("Serial0 Screen initialization failed!");
    #endif

    screenWorking = 0;  //screen is NOT working flag
  }
  else{
    screenWorking = 1;  //screen IS working flag

    //gfx->setRotation(1);  //If using other gfx libaries prob need to do this. You have to use one or the other unfortuantly currently as setting this breaks what the guy did to get the Arduino_Canvas working on this board
    //curTestPanel = "Panel1";
    curTestPanel = "Test";    //default to test mode
    displayScreen(1);
    screen.flush(); //update display HAVE TO DO THIS ANYTIME YOU WANT THE SCREEN TO UPDATE! 
    curTestPanel = "Panel1";  //change us to a non test background after any following buttons
  }
}

//Note the touchscreen on this unit doesnt seem to be enabled around the outside edge of the display
void handleTouchScreen(){

  //check if there was a touch interrupt. If no touch then immediatly return
  #ifdef USING_TOUCH_INT
    if (!screen.screenWasTouched ){ return; }      //THIS WORKS! Got the touch interrupt working
    screen.nextTouchScreenCheck = cur_millis + 30; //arm interrupt 30ms later? Prob dont really need this
  #else
    if (!screen.screenWasTouched && cur_millis < screen.nextTouchScreenCheck ){ return; }   //this works and is a fallback if the interrupt pin is bad
    else { screen.nextTouchScreenCheck = cur_millis + 30; }  //check every 30ms
  #endif

  if (!screenWorking){ return; }

  //Check if the display was touched. touchX & touchY are GLOBAL VARS. This does a few i2c transactions that could take some time and potentially screw up network functions. 
  //May  only want to check this like once every 10ms maybe?
  if (screen.getTouchPoint(touchX, touchY)){
    LastTouch.Text = "LT=" + clockString;         //update time on button but dont automatically re-display button
    //LastTouch.updateText("LT=" + clockString);  //update text and display button

    // HANDLE CLICKS FOR Panel1 or Test
    if (curTestPanel == "Panel1" || curTestPanel == "Test" )
    {
      //check if our last touch clock button was clicked
      if (LastTouch.checkIfClicked(touchX, touchY)){
        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,100,100);       //set text color
        screen.prt("Last touch button clicked",0,0,2); //print touch coords
        delay(100);
        #ifdef USE_HID_KEYBOARD
          //Keyboard.write('\n'); 
          //Keyboard.print("https://google.com");
          Keyboard.println("https://google.com");
        #endif
      }

      //check if our clear screen button was clicked
      else if (CS.checkIfClicked(touchX, touchY)){
        screen.clear(0,0,0);                //black out screen
        displayPanel1Buttons(1);

        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,255,255);       //set text color
        screen.prt("Screen Cleared",0,0,3); //print touch coords
        delay(100);
      }
      else if (ColorRand.checkIfClicked(touchX, touchY)){
        uint8_t randR = random(0, 255);
        uint8_t randG = random(0, 255);
        uint8_t randB = random(0, 255);
        screen.clear(randR,randG,randB);    //set random color
        displayScreen(1);

        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,100,100);       //set text color
        screen.prt("Random Background Color",0,0,2);   //print touch coords
        screen.prt("RGB=" + String(randR) + "," + String(randG) + "," + String(randB),0,20,2);   //print touch coords
        delay(100);
      }
      else if (ListSdFILES.checkIfClicked(touchX, touchY)){
        screen.clear(0,0,0);    //set random color
        displayScreen(0);

        screen.setColor(50,50,50);            //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
        screen.setColor(255,100,100);         //set text color
        screen.prt("Listing SD Files",0,0,2);  //print touch coords
        screen.prt("Bootcnt=" + String(sdBootCounter),0,20,2);  //print touch coords

        screen.setColor(255,255,255);          //set background color of rectangle
        Serial.println("Showing SD files");
        if (screenWorking){
          showSdFilesOnLCD();
        }
        delay(100);
      }
      else if (TestModeBut.checkIfClicked(touchX, touchY)){
        //ButtonGuiClass TestModeBut  (screenPtr, 325, 160, 150, 28, 255, 255, 255, "TestMode Off", 2);  //PANEL 1 button to make a random background color
        if (TestModeBut.Text == "TestMode Off"){
          curTestPanel = "Test";
          TestModeBut.Text = "TestMode On";
        }
        else{
          curTestPanel = "Panel1";
          TestModeBut.Text = "TestMode Off";
        }

        screen.clear(0,0,0);    //set random color
        screen.setColor(50,50,50);            //set background color of rectangle

        displayScreen(1);
        delay(100);
      }
      //else test for other touches on touchscreen if in test mode and show keypresses
      else if (TestModeBut.Text == "TestMode On"){
        Serial.println("Touch Pressed:" + String(touchX) + "," + String(touchY));
        screen.setColor(0,50,100);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //we are 480 x 80

        screen.setColor(200,255,255);       //set text color
        screen.prt((String(touchX) + "," + String(touchY)),0,0,5);    //print touch coords

        screen.setColor(150,255,255);        //set color of circle
        screen.drawCircleOutline(touchX, touchY, 4);  //print tiny circle where touch happened
      }
    }

    // HANDLE CLICKS FOR Panel2
    else if (curTestPanel == "Panel2"){
      if (ResetChip.checkIfClicked(touchX, touchY)){
        resetEsp32();
      }
      else if (ResetToBL.checkIfClicked(touchX, touchY)){
        reboot_to_bootloader();   //reboot to bootloader
      }
    }

    checkPanelChangeButtons();  //do this for all cases as these buttons should be on every screen
    screen.flush();             //for all cases after a touch write to screen

    #ifndef USING_TOUCH_INT
      //prob only need this extra delay and clearing the touch data if we are polling
      delay(20);                  //Try to prevent multiple touches with a dumb delay
      screen.clearTouchData();    //Try to prevent multiple touches after a delay by clearing touch data
    #endif

    screen.screenWasTouched = 0;       //clear touch interrupt variable in case it was hit during the above delays
  } //end if touch 
}

//screen flush happens at the end of handleTouchScreen function. touchX & touchY are GLOBAL VARS
void checkPanelChangeButtons(){
  if (Panel1Button.checkIfClicked(touchX, touchY)){
    curTestPanel = "Panel1";
    screen.clear(5, 50, 50); //clear background to a dark blue
    //screen.flush();   //for all cases after a touch write to screen
    displayScreen(1);
    delay(50);
  }
  else if (Panel2Button.checkIfClicked(touchX, touchY)){
    curTestPanel = "Panel2";
    screen.clear(0, 0, 0); //clear background to BLACK
    //screen.flush();   //for all cases after a touch write to screen
    displayScreen(1);
    delay(50);
  }
}


void displayScreen(int whatToDisplay){
  if (!screenWorking){ return; }

  if (curTestPanel == "Panel1"){
    panel1Display(whatToDisplay);
  }
  else if (curTestPanel == "Panel2"){
    panel2Display(whatToDisplay);
  }
  //for unknown case show test screen
  else{
    displayTestScreen();
  }
}

void panel1Display(int whatToDisplay){
  screen.setColor(100,255,255);       //set text color
  screen.prt("Panel 1",0,0,3);   //print touch coords

  screen.setColor(255,255,255);       //set text color
  displayPanel1Buttons(whatToDisplay);

  //if we want a clear display pass 0
  if (whatToDisplay > 0){
    printWifiInfo();
    //readBatteryVoltage(1);
    //screen.prt("BattVoltage =" + String(batteryVoltage,3)          +  " V"   ,0,240,2);   //
    //screen.prt("StartVoltage=" + String(batteryVoltageAtStartup,3) +  " V"   ,0,260,2);   //
  }
}

void displayPanel1Buttons(int whatToDisplay){
  LastTouch.displayButt();            //display the button
  CS.displayButt();                   //redisplay button
  ColorRand.displayButt();            //redisplay button
  ListSdFILES.displayButt();
  TestModeBut.displayButt();
  printCurrentClock();                //display clock1
  Panel1Button.displayButt();
  Panel2Button.displayButt();
}

void panel2Display(int whatToDisplay){
  screen.setColor(100,255,255);       //set text color
  screen.prt("Admin Settings",0,0,3);   //print touch coords

  screen.setColor(255,255,255);       //set text color white
  screen.prt("Admin Data 1=" + String(random(0,10)),0,40,2);   //print touch coords
  screen.prt("Admin Data 2=" + String(random(0,1000)),0,60,2);   //print touch coords
  screen.prt("Admin Data 3=" + String(random(0,1000)),0,80,2);   //print touch coords
  screen.prt("Admin Data 4=" + String(random(0,1000)),0,100,2);   //print touch coords
  screen.prt("Admin Data 5=" + String(random(0,1000)),0,120,2);   //print touch coords

  ResetChip.displayButt();            //redisplay button
  ResetToBL.displayButt();            //redisplay button
  printCurrentClock();                //display clock
  Panel1Button.displayButt();
  Panel2Button.displayButt();
}

void displayTestScreen(){
  screen.clear(5, 70, 70); //set background to be dark teal

  screen.setColor(255,255,255);                       //set text color
  //screen.setFont(&Roboto_Regular16pt7b);              //use roboto font. WARNING THIS IS A FIXED LARGE FONT
  //screen.setFont(NULL);                               //revert to original font?

  screen.prt("Program Ver=" + programVersion,0,0,3);  //print text
  screen.setColor(255,100,100);      //set text color
  screen.prt("Font size-1",0,40,1);  //print text
  screen.prt("Font size-2",0,60,2);  //print text
  screen.prt("Font size-3",0,80,3);  //print text
  screen.prt("Font size-4",0,110,4); //print text
  screen.prt("Font size-5",0,140,5); //print text

  screen.setColor(10,90,100);                                    //set background color of rectangle
  screen.drawFillRect(0, 210, screen.width, screen.height - 210); //clear the status bar
  screen.setColor(50,255,255);               //set text color
  screen.prt("Click on buttons or screen to test functions. Testing the wrap around functon on screen. It should auto wrap",0,220,2);  //print text
  
  TestModeBut.Text = "TestMode On";
  displayPanel1Buttons(1);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //screen.flush(); //update display HAVE TO DO THIS ANYTIME YOU WANT THE SCREEN TO UPDATE! 
}

//should just use the button class for this and update the text
void printCurrentClock(){
  // clear time background
  screen.setColor(0,0,0);               //set background color of rectangle
  screen.drawFillRect(325, 2, 150, 35); //clear the bar

  //write the time
  screen.setColor(40,255,255);       //set background color of text
  screen.prt(clockString,335,12,2);   //print clock
}

void displayUptime(){
  cur_millis = millis();
  if (cur_millis >= targetTime)
  {
    targetTime += 1000;
    ss++; // Advance second
    if (ss == 60)
    {
      ss = 0;
      mm++; // Advance minute
      if (mm > 59)
      {
        mm = 0;
        hh++; // Advance hour
        if (hh > 23)
        {
          hh = 0;
        }
      }
    }

    clockString = "";
    if (hh < 10) {
      clockString = clockString + "0";
    }

    clockString = clockString + hh + ":";
    if (mm < 10) {
      clockString = clockString + "0";
    }

    clockString = clockString + mm + ":";
    if (ss < 10) {
      clockString = clockString + "0";
    }
    clockString = clockString + ss;

    //every 5 seconds output uptime to serial 
    if (ss % 5 == 0){
      Serial.print(clockString + " ");
      Serial0.println("Serial0-" + clockString);
      readBatteryVoltage(1);
      screen.setColor(0,0,0);               //set background color of rectangle
      screen.drawFillRect(0, 240, 240, 40); //we are 480 x 80
      screen.setColor(200,255,255);         //set background color of rectangle
      screen.prt("BattVoltage =" + String(batteryVoltage,3)          +  " V"   ,0,240,2);   //
      screen.prt("StartVoltage=" + String(batteryVoltageAtStartup,3) +  " V"   ,0,260,2);   //
    }

    if (!screenWorking){
      return;
    }
    else{
      printCurrentClock();
      screen.flush();
    }
  }
} //end displayUptime

void pressControlKey(char whichChar){
  #ifdef USE_HID_KEYBOARD
    //enter control alt delete
    Keyboard.press(KEY_LEFT_CTRL);
    //Keyboard.press(KEY_LEFT_ALT);
    //Keyboard.press(KEY_DELETE);
    delay(100);
    //Keyboard.press('c');
    Keyboard.press(whichChar);
    
    Keyboard.releaseAll();
    Keyboard.flush();
    delay(50);
  #else
    Serial.println("USE_HID_KEYBOARD not defined in .ino file");
  #endif
}

//shut down periphials. Mainly needed for bootloader mode maybe?
void getChipReadyForRestart(){

  #ifdef USE_FTP_SERVER
    if (ftpStarted) {
      ftpSrv.end(); //may be killing us?
    }
  #endif

  SD_MMC.end(); //safley shut down SD card when done with it?
  //screen.end();
  //Keyboard.end();

  //prob un-necessary to do some of this stuff
  if (WiFi.status() == WL_CONNECTED){ 
    //Serial.println("Disconnecting WiFi...");
    //WiFi.disconnect(true);  // true = erase credentials
    //WiFi.mode(WIFI_OFF);    // Turn off radio
    delay(100);
  }

  Serial.println("Done with getChipReadyForRestart");
  delay(50);
  Serial.flush();
}

void showSdFilesOnLCD(){
  listDir(SD_MMC, "/", 0, 1);
}

void incrementBootCounterOnSdCard(){
  //first make sure card exists
  if (cardType != CARD_NONE) {

    // Open File
    File file = SD_MMC.open("/bootCnt.txt");
    if (!file) {
      Serial.println("Failed to open bootCnt.txt");
      writeFile(SD_MMC, "/bootCnt.txt", "1");       //create file if it doesnt exist
      return;
    }

    // Read File and Convert to Integer
    else if (file.available()) {
      String data = file.readString();  // Read text string
      Serial.println("bootCnt initial value=" + String(data));

      sdBootCounter = data.toInt();         // Convert to integer [5] 
      sdBootCounter = sdBootCounter + 1;                //increment boot counter
      //appendFile(SD_MMC, "/bootCounter.txt", String(value) + "\n");
      writeFile(SD_MMC, "/bootCnt.txt", String(sdBootCounter).c_str());
    }
    else{
      writeFile(SD_MMC, "/bootCnt.txt", "1");       //create file if its not availiable. This prob shouldnt be hit
    }

    file.close(); //always try to close file when done writing to try and ensure it doesnt get lost if power is lost
  }
  else{
    Serial.println("No SD_MMC card attached");
  }
}

void initSDcard(){
  esp_rom_printf("Initialize tf card\n");
  //SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);

  if(! SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0)){
      Serial.println("SD_MMC.setPins change failed!");
      return;
  }
  else{
    Serial.println("SD_MMC.setPins success!");
  }

  //bool SDMMCFS::begin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed, int sdmmc_frequency, uint8_t maxOpenFiles) 
  //if (!SD_MMC.begin()) 
  //if (!SD_MMC.begin("/sdcard", true)){
  //if (!SD_MMC.begin("/sdmmc", true, false, 20000))    //original from vendor
  if (!SD_MMC.begin("/sdcard", true, false, 20000))     //this works. starts it up in 1 bit MMC mode
  {
    esp_rom_printf("Card Mount Failed\r\n");
    return;
  }
  else{
    cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE) {
      Serial.println("No SD_MMC card attached");
      return;
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\r\n", cardSize);
    Serial.printf("Total space: %lluMB\r\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used space : %lluMB\r\n", SD_MMC.usedBytes() / (1024 * 1024));

    listDir(SD_MMC, "/", 0, 0);
    /*
    //dont do extra SD cards tests every time. But the following was working when uncommenting
    createDir(SD_MMC, "/mydir");
    listDir(SD_MMC, "/", 0, 0);
    removeDir(SD_MMC, "/mydir");
    listDir(SD_MMC, "/", 2, 0);
    writeFile(SD_MMC, "/hello.txt", "Hello ");
    appendFile(SD_MMC, "/hello.txt", "World!\n");
    readFile(SD_MMC, "/hello.txt");
    deleteFile(SD_MMC, "/foo.txt");
    renameFile(SD_MMC, "/hello.txt", "/foo.txt");
    readFile(SD_MMC, "/foo.txt");
    testFileIO(SD_MMC, "/test.txt");
    */

    //SD_MMC.end(); //safley shut down SD card when done with it?
  }
}

void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Split the command by delimiter '|'
    int params[10];  // Array to store numeric parameters
    String textParam = "";  // For text parameter
    String parts[11];  // To store parts of the command
    
    int partCount = 0;
    int startIndex = 0;
    int delimiterIndex;
    
    while ((delimiterIndex = command.indexOf('|', startIndex)) != -1 && partCount < 11) {
      parts[partCount++] = command.substring(startIndex, delimiterIndex);
      startIndex = delimiterIndex + 1;
    }
    
    // Add the last part
    if (startIndex < command.length() && partCount < 11) {
      parts[partCount++] = command.substring(startIndex);
    }
    
    if (partCount < 1) {
      Serial.println("Invalid command format");
      return;
    }
    
    String cmd = parts[0];
    
    // Process based on command
    if (cmd == "prt" && partCount >= 4) {
      textParam = parts[1];
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+2].toInt();
      }
      screen.prt(textParam, params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Text printed: " + textParam);
    }
    else if (cmd == "clear" && partCount >= 1) {
      if (partCount >= 4) {
        for (int i = 0; i < 3; i++) {
          params[i] = parts[i+1].toInt();
        }
        screen.clear(params[0], params[1], params[2]);
      } else {
        screen.clear(); // Use default values
      }

      screen.flush();
      Serial.println("Screen cleared");
    }
    else if (cmd == "setColor" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.setColor(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Color set");
    }
    else if (cmd == "drawQRCode" && partCount >= 11) {
      textParam = parts[1];
      for (int i = 0; i < 9; i++) {
        params[i] = parts[i+2].toInt();
      }
      screen.drawQRCode(textParam.c_str(), params[0], params[1], params[2], 
                        params[3], params[4], params[5], params[6], params[7], params[8]);
      screen.flush();
      Serial.println("QR code drawn");
    }
    else if (cmd == "drawFillRect" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillRect(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Rectangle filled");
    }
    else if (cmd == "drawRect" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawRect(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Rectangle drawn");
    }
    else if (cmd == "drawLine" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawLine(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Line drawn");
    }
    else if (cmd == "drawFillCircle" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillCircle(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Circle filled");
    }
    else if (cmd == "drawCircleOutline" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawCircleOutline(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Circle outline drawn");
    }
    else if (cmd == "drawTriangle" && partCount >= 7) {
      for (int i = 0; i < 6; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawTriangle(params[0], params[1], params[2], params[3], params[4], params[5]);
      screen.flush();
      Serial.println("Triangle drawn");
    }
    else if (cmd == "drawFillTriangle" && partCount >= 7) {
      for (int i = 0; i < 6; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillTriangle(params[0], params[1], params[2], params[3], params[4], params[5]);
      screen.flush();
      Serial.println("Triangle filled");
    }
    else if (cmd == "drawRoundRect" && partCount >= 6) {
      for (int i = 0; i < 5; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawRoundRect(params[0], params[1], params[2], params[3], params[4]);
      screen.flush();
      Serial.println("Rounded rectangle drawn");
    }
    else if (cmd == "drawFillRoundRect" && partCount >= 6) {
      for (int i = 0; i < 5; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillRoundRect(params[0], params[1], params[2], params[3], params[4]);
      screen.flush();
      Serial.println("Rounded rectangle filled");
    }
    else if (cmd == "drawEllipse" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawEllipse(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Ellipse drawn");
    }
    else if (cmd == "drawFillEllipse" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillEllipse(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Ellipse filled");
    }
    else if (cmd == "flush") {
      screen.flush();
      Serial.println("Screen flushed");
    }
    else {
      Serial.println("Unknown or incomplete command: " + cmd);
    }
  }
}

// FTP CALLBACKS
// Callbacks (avoid leading underscore names to reduce linter warnings)
void ftpStatusCallback(FtpOperation ftpOperation, uint32_t freeSpace, uint32_t totalSpace) {
  switch (ftpOperation) {
    case FTP_CONNECT:
      Serial.println(F("FTP: Connected!"));
      break;
    case FTP_DISCONNECT:
      Serial.println(F("FTP: Disconnected!"));
      break;
    case FTP_FREE_SPACE_CHANGE:
      Serial.printf("FTP: Free space change, free %u of %u!\n", freeSpace, totalSpace);
      break;
    default:
      break;
  }
}

void ftpTransferCallback(FtpTransferOperation ftpOperation, const char* name, uint32_t transferredSize) {
  switch (ftpOperation) {
    case FTP_UPLOAD_START:
      Serial.println(F("FTP: Upload start!"));
      break;
    case FTP_UPLOAD:
      Serial.printf("FTP: Upload of file %s byte %u\n", name, transferredSize);
      break;
    case FTP_TRANSFER_STOP:
      Serial.println(F("FTP: Finished transfer!"));
      break;
    case FTP_TRANSFER_ERROR:
      Serial.println(F("FTP: Transfer error!"));
      break;
    default:
      break;
  }
}

/////////////////////////////// SD FUNCTIONS
void listDir(fs::FS &fs, const char *dirname, uint8_t levels, bool printOnLCD) {
  //screen.setColor(255,255,255);       //set text color white
  int currentLcdLine = 45;
  String dataLine;
  char dateTimeBuff[12]; // Buffer to store the formatted file datetime string. Should only be 10 bytes plus null string

  Serial.printf("Listing directory: %s\r\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    if (printOnLCD){
      screen.prt("Failed to open directory",0,currentLcdLine,2);   //print touch coords
    }
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    if (printOnLCD){
      screen.prt("Not a directory",0,currentLcdLine,2); 
    }
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("DIR : ");
      Serial.println(file.name());
      if (printOnLCD){
        screen.prt("DIR:" + String(file.name()), 0, currentLcdLine, 1);
        currentLcdLine = currentLcdLine + 10;
      }

      if (levels) {
        listDir(fs, file.path(), levels - 1, printOnLCD);
      }
    } else {
      /*
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      */
      //try to show file time along with filename and date
      time_t t = file.getLastWrite(); // Verify what time the library thinks it is
      struct tm * tmstruct = localtime(&t);

      //Serial.printf("  File: %s  Size: %d Date: %d-%02d-%02d\n", file.name(), file.size(), (tmstruct->tm_year)+1900, (tmstruct->tm_mon)+1, tmstruct->tm_mday);
      snprintf(dateTimeBuff, sizeof(dateTimeBuff), "%d-%02d-%02d", (tmstruct->tm_year)+1900, (tmstruct->tm_mon)+1, tmstruct->tm_mday);
      
      dataLine = " " + String(file.name()) + " Size:" + String(file.size()) + " " + String(dateTimeBuff);
      Serial.println(dataLine);

      if (printOnLCD){
        //screen.prt(String(file.name()) + " size=" + String(file.size()) ,5,currentLcdLine,1); 
        screen.prt(dataLine ,5,currentLcdLine,1);
        currentLcdLine = currentLcdLine + 10;
      }
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\r\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\r\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%zu bytes read for %lu ms\r\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %lu ms\r\n", 2048 * 512, end);
  file.close();
}


void initArduinoOTA(bool forceSetup){
  if (WiFi.status() == WL_CONNECTED){ 
    if (!otaSetup || forceSetup)
    {
      Serial.println("Setting up Arduino OTA");

      // Port defaults to 3232
      //ArduinoOTA.setPort(3232);

      // Hostname defaults to esp3232-[MAC]
      ArduinoOTA.setHostname(esp32Hostname);

      // Password can be set with plain text (will be hashed internally)
      // The authentication uses PBKDF2-HMAC-SHA256 with 10,000 iterations
      ArduinoOTA.setPassword("admin");

      // Or set password with pre-hashed value (SHA256 hash of "admin")
      // SHA256(admin) = 8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918
      // ArduinoOTA.setPasswordHash("8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918");

      if (!MDNS.begin(esp32Hostname)) {
        Serial.println("Error setting up MDNS responder!");
      }
      else{
        Serial.println("Started MDNS responder!");
      }

      ArduinoOTA
        .onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
          } else {  // U_SPIFFS
            type = "filesystem";
          }

          //isOtaHappening = 1;
          // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
          Serial.println("Start updating " + type);
        })
        .onEnd([]() {
          Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
          if (millis() - last_ota_time > 500) {
            Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
            last_ota_time = millis();
          }
        })
        .onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
          } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
          } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
          } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
          } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
          }
        });

      Serial.println("Running ArduinoOTA.begin()");
      ArduinoOTA.begin();
      otaSetup = 1;
    }
  }
  else{
    //Serial.println("Wifi not connected. Not starting Arduino OTA");
  }
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time from timeserver");
    return;
  }
  else{
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
}

void syncTimeToNTP(bool forceSync){
  if (!ntpSynced || forceSync){

    if (WiFi.status() == WL_CONNECTED){ 
      // Sync time with NTP server using basic setup
      //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      // Set timezone and sync with NTP
      configTzTime(posixTZ, ntpServer);

      // Wait until time is synced
      Serial.print("Syncing time. ");
      struct tm timeinfo;
      while (!getLocalTime(&timeinfo)) {
        delay(500); Serial.print(".");
      }

      ntpSynced = true;
      Serial.println("Time synced!");
      delay(10);
      printLocalTime();
    }
  }
}

void setMacAddressToBench(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("Original MAC=%02x:%02x:%02x:%02x:%02x:%02x  ", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    
    //set our mac to be custom
    baseMac[0] = customMacFirst3Bytes[0];
    baseMac[1] = customMacFirst3Bytes[1];
    baseMac[2] = customMacFirst3Bytes[2];

    // Change ESP32 Station Mac Address
    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &baseMac[0]);
    //esp_err_t err = esp_wifi_set_mac(SOFTAP_IF, &newMACAddress[0]);   //if using softAP need to do this
    if (err == ESP_OK) {
      Serial.println("Successfully changed MAC");
    }
    else{
      Serial.println("Error trying to change Mac Address");
    }
  } 
  else {
    Serial.println("Failed to read MAC address. Not trying to change");
  }
}

void printResetReason(){
  esp_reset_reason_t theResetReason = esp_reset_reason();
  String resetReasonStr = "Reset Reason=";

  switch (theResetReason) {
    case ESP_RST_POWERON:  resetReasonStr = resetReasonStr + "Power-on reset"; break;
    case ESP_RST_EXT:      resetReasonStr = resetReasonStr + "Reset by external pin"; break;
    case ESP_RST_SW:       resetReasonStr = resetReasonStr + "Software reset (esp_restart)"; break;
    case ESP_RST_PANIC:    resetReasonStr = resetReasonStr + "Exception/panic reset"; break;
    case ESP_RST_INT_WDT:  resetReasonStr = resetReasonStr + "Interrupt Watchdog reset"; break;
    case ESP_RST_TASK_WDT: resetReasonStr = resetReasonStr + "Task Watchdog reset"; break;
    case ESP_RST_WDT:      resetReasonStr = resetReasonStr + "Other Watchdog reset"; break;
    case ESP_RST_DEEPSLEEP:resetReasonStr = resetReasonStr + "Wakeup from deep sleep"; break;
    case ESP_RST_BROWNOUT: resetReasonStr = resetReasonStr + "Brownout reset"; break;
    case ESP_RST_SDIO:     resetReasonStr = resetReasonStr + "Reset over SDIO"; break;
    default:               resetReasonStr = resetReasonStr + "Other/Unknown"; break;
  }

  Serial.println("\r\n-----------------------------------------------------------------------");
  Serial.println(resetReasonStr);
  #ifdef USE_SERIAL0
    Serial0.println("\r\n-----------------------------------------------------------------------");
    Serial0.println(resetReasonStr);
  #endif
}

void initWifi(bool forceSetup){

  if (WiFi.status() != WL_CONNECTED || forceSetup){ 
    //setup wifi
    WiFi.mode(WIFI_STA);
    //WiFi.setHostname(esp32Hostname); 
    setMacAddressToBench();

    //connect to wifi
    Serial.print("Connecting to WiFi=" + String(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiStarted = true;

    // Wait for connection
    uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - started) < 20000) {
      delay(500);
      Serial.print('.');
    }
  }

  if (WiFi.status() == WL_CONNECTED){ 
    Serial.print("\r\nIP address       : ");
    Serial.println(WiFi.localIP()); // <--- This prints the DHCP IP
    Serial.print("Subnet Mask      : ");
    Serial.println(WiFi.subnetMask());
    Serial.print("ESP32 MAC Address: ");
    Serial.println(WiFi.macAddress());

    displayWifiInfoInTopLeftCorner();
  }
  else{
    Serial.println("\r\nCould not connect to WIFI");
  }
}

void initFtpServer(){
  #ifdef USE_FTP_SERVER
    Serial.println("SimpleFTP STORAGE_TYPE=" + String(STORAGE_TYPE));
    if (STORAGE_TYPE == STORAGE_SD_MMC )
    {
      Serial.println("SimpleFTP Storage type is in SD_MMC mode");
    }
    else{
      Serial.println("SimpleFTP Storage type is NOT in SD_MMC mode. This is bad");
    }

    //start FTP server if we have a sdcard attached
    if (cardType != CARD_NONE) {
      // setup callbacks and start ftp server
      ftpSrv.setCallback(ftpStatusCallback);
      ftpSrv.setTransferCallback(ftpTransferCallback);  

      Serial.println("Starting FTP server...");
      ftpSrv.begin(FTP_USER, FTP_PASS);
      ftpStarted = true;  //this seems like an uncessesary state variable
    }
  #endif
}

String getCurrentRTCTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Failed to obtain time";
    }
    
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeString);
}

void displayWifiInfoInTopLeftCorner(){
  if (screenWorking)
  {
    screen.setColor(50,50,50);            //set background color of rectangle
    screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
    screen.setColor(255,255,255);         //set text color
    screen.prt("IP =" + WiFi.localIP().toString(),0,0,2);  //print touch coords
    screen.prt("Mac=" + String(WiFi.macAddress()),0,20,2);  //print touch coords
  }
}

void printWifiInfo(){
  if (screenWorking)
  {
    screen.setColor(255,255,255);       //set text color

    if (WiFi.status() == WL_CONNECTED){ 
      screen.prt("Wifi connected to " + String(WIFI_SSID),0,40,2);   //
    }
    else{
      screen.prt("Wifi Not connected to "+ String(WIFI_SSID),0,40,2);   //
    }

    screen.prt("IP  =" + WiFi.localIP().toString(),0,60,2);   //
    screen.prt("Mac =" + String(WiFi.macAddress()),0,80,2);   //
    screen.prt("Time=" + getCurrentRTCTime()      ,0,100,2);   //
  }
}

void printDebugInfo(){
  #ifdef PRINT_SKETCH_DEBUG
    // Print available serial commands
    Serial.println("Available commands (format: command|param1|param2|...):");
    Serial.println("  prt|text|x|y|size");
    Serial.println("  clear|r|g|b");
    Serial.println("  setColor|r|g|b");
    Serial.println("  drawQRCode|data|x|y|moduleSize|bgR|bgG|bgB|fgR|fgG|fgB");
    Serial.println("  drawFillRect|x|y|w|h");
    Serial.println("  drawRect|x|y|w|h");
    Serial.println("  drawLine|x0|y0|x1|y1");
    Serial.println("  drawFillCircle|x|y|radius");
    Serial.println("  drawCircleOutline|x|y|radius");
    Serial.println("  drawTriangle|x0|y0|x1|y1|x2|y2");
    Serial.println("  drawFillTriangle|x0|y0|x1|y1|x2|y2");
    Serial.println("  drawRoundRect|x|y|w|h|radius");
    Serial.println("  drawFillRoundRect|x|y|w|h|radius");
    Serial.println("  drawEllipse|x|y|rx|ry");
    Serial.println("  drawFillEllipse|x|y|rx|ry");
    Serial.println("  flush");
  #endif
  Serial.println("Serial interface ready!");
  Serial0.println("Serial0 interface ready!");
}

void initKeyboard(){
  // initialize control over the keyboard:
  #ifdef USE_HID_KEYBOARD
    Keyboard.begin();
    USB.begin();
    delay(1000);
  #else
    #if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 0)
      Keyboard.begin();
      USB.begin();
      Serial.println("\r\nUSE_HID_KEYBOARD not being used but TinyUSB is enabled so start USB");
      Serial0.println("\r\nSerial0 USE_HID_KEYBOARD not being used but TinyUSB is enabled so start USB");
    #else
      Serial.println("\r\nUSE_HID_KEYBOARD not being used");
      Serial0.println("\r\nSerial0 USE_HID_KEYBOARD not being used");
    #endif 
  #endif
}

/*
//need to include this if using this function #include "soc/timer_group_reg.h"
void reboot_to_bootloaderNUKE(){
  // 1. Set the Download Boot flag in RTC memory
  WRITE_PERI_REG(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);

  // If a normal restart is hanging, we force a watchdog timeout
  //*((uint32_t *)0x60000000) = 0; // Intentional null pointer/illegal access to crash

  // 2. Enable the Watchdog Timer to trigger an immediate reset
  // This feeds the hardware "reset" line directly
  WRITE_PERI_REG(TIMG_WDTCONFIG0_REG_W0, 0); // Disable protection
  WRITE_PERI_REG(TIMG_WDTCONFIG0_REG_W0, TIMG_WDT_FLASHBOOT_MOD_EN_S | TIMG_WDT_SYS_RESET_LENGTH_S | TIMG_WDT_CPU_RESET_LENGTH_S | TIMG_WDT_EN_S);
  WRITE_PERI_REG(TIMG_WDTFEED_REG_W0, 1); // Trigger!

  // The chip will reset here.
  while(1);
}
*/

// This is the recommended way to reboot into the serial bootloader
void reboot_to_bootloader() {
  if (screenWorking){
    screen.clear(0,0,0);                //black out screen
    screen.setColor(50,50,50);            //set background color of rectangle
    screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
    screen.setColor(255,100,100);         //set text color
    screen.prt("Entering Bootloader",0,0,2);  //print touch coords

    screen.setColor(255,255,255);         //set text color
    screen.prt("Esp32 will reboot into bootloader mode and provide an additonal serial port to use to flash device",0,40,2);  //print touch coords
    screen.flush();   //anytime you want the screen to update you have to do a flush
  }

  getChipReadyForRestart(); //may not really needed
  delay(100);

  // 2. Give the serial buffer a moment to breathe
  Serial.println("Rebooting to Download Mode...");
  Serial.flush();
  detachInterrupt(digitalPinToInterrupt(TOUCH_PIN_NUM_INT));    //remove touch interrupt
  delay(100);
  
  // This sets the RTC_CNTL_FORCE_DOWNLOAD_BOOT bit (bit 1) 
  // in the RTC_CNTL_OPTION1_REG register.
  WRITE_PERI_REG(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);

  // 2. Use the internal 'instant' reset if available
  // This bypasses the standard esp_restart() hooks
  esp_reset_reason_t reason = esp_reset_reason();

  // Now restart. The chip will see this bit and jump to the bootloader.
  esp_restart();  //this isnt restarting properly
  //ESP.restart();    //new callrestart esp32

    //if we get this far just try and stay to potentially handle the OTA
  while(1){
    ArduinoOTA.handle();
  }
}

void resetEsp32(){
  if (screenWorking){
    screen.setColor(50,50,50);            //set background color of rectangle
    screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
    screen.setColor(255,100,100);         //set text color
    screen.prt("Resetting esp32",0,0,3);  //print touch coords

    screen.flush();   //anytime you want the screen to update you have to do a flush
  }

  getChipReadyForRestart();
  delay(50);
  ESP.restart();    //restart esp32
}

void handleNetworkTasks(){
  //ArduinoOTA.handle();      //run regardless of wifi connection?

  if (WiFi.status() == WL_CONNECTED){ 
    ArduinoOTA.handle();      //run only if wifi is connected?

    #ifdef USE_FTP_SERVER
      if (ftpStarted) {
        ftpSrv.handleFTP();
      }
      else if (cardType != CARD_NONE) {
        //Restart FTP server
        initFtpServer();
      }
    #endif

    //dedicate CPU to just handling OTA only once started to avoid OTA issue
    while (isOtaHappening){
        ArduinoOTA.handle();
    }
  }

  //periodically try to reinit failed wifi setup? Fairly sure this is intellengenly handled alredy
  else if (cur_millis > nextNetworkConfigCheck)
  {
    nextNetworkConfigCheck = cur_millis + 30000;
    initWifi(0);
    syncTimeToNTP(0);
    initArduinoOTA(0);
  }
}