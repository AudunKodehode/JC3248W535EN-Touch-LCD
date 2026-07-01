// WiFi credentials (replace before use)
const char* WIFI_SSID = "CHANGE-THIS";
const char* WIFI_PASSWORD = "CHANGE-THIS";
const char* esp32Hostname = "esp32s3";

// FTP credentials (replace before use)
const char* FTP_USER = "user";
const char* FTP_PASS = "password";

const char* ntpServer  = "pool.ntp.org";  //NTP server
//const long  gmtOffset_sec = -8 * 60 * 60; // Adjust for your timezone This value represents the 8-hour offset behind UTC/GMT (seconds). PST time (-8) is -28800
//const int   daylightOffset_sec = 3600;
const char* posixTZ     = "PST8PDT,M3.2.0,M11.1.0";  // Pacific Time with auto DST

//we change the first 3 bytes in the wifi MAC to this
const uint8_t customMacFirst3Bytes[3] = {0x11, 0x22, 0x33};

#define USE_FTP_SERVER
#define USE_SERIAL0
//#define USE_HID_KEYBOARD    //If this flag is set then use the USB keyboard
//#define PRINT_SKETCH_DEBUG