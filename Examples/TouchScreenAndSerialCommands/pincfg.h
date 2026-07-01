#ifndef _PINCFG_H_
#define _PINCFG_H_

//https://www.letscontrolit.com/forum/viewtopic.php?t=10480
#define TFT_BLK_ON_LEVEL 1    // GPIO1 Goes to pin36 on LCD connector for "LED_K" whatever that means
#define TFT_BLK 1     // GPIO1 Goes to pin36 on LCD connector for "LED_K" whatever that means
#define TFT_RST -1    //-1 ?? There is no GPIO dedicated to the LCD Reset it looks like. Goes straight to the RST line on the esp32 EN pin
#define TFT_CS 45
#define TFT_SCK 47
#define TFT_SDA0 21
#define TFT_SDA1 48
#define TFT_SDA2 40
#define TFT_SDA3 39
#define TFT_TE 38

#define TOUCH_PIN_NUM_I2C_SCL 8
#define TOUCH_PIN_NUM_I2C_SDA 4
#define TOUCH_PIN_NUM_INT 3       //Called TP_INT now getting interrupt from touches instead of constantly polling i2c to detect touches
#define TOUCH_PIN_NUM_RST -1      //-1 ??

#define SD_MMC_D0 13    //Also MCU_MISO
#define SD_MMC_CLK 12   //Also TF_CLK
#define SD_MMC_CMD 11   //Also MCU_MOSI
#define TF_CS 10        //This is not used for the SD_MMC library but may be needed for other SD libraries

#define AUDIO_I2S_PORT I2S_NUM_0
#define AUDIO_I2S_MCK_IO -1   // MCK
#define AUDIO_I2S_BCK_IO 42   // BCK
#define AUDIO_I2S_LRCK_IO 2   // LCK
#define AUDIO_I2S_DO_IO 41    // DIN

#define BAT_ADC_PIN 5   //can read the voltage of the battery here. There is a resistor dividor that prob maps it to 0-1V

//Header P1 is JST1.25 4P Power and UART header
//#define HeaderP1_4PIN_PIN1_VIN     //This is 5V? Be carefull
#define HeaderP1_4PIN_PIN2_TXD2 43   //Hardline UART TX pin 37 TXD0 (UART0 TX): GPIO 43. Looks to be defined as "TX" in arduino
#define HeaderP1_4PIN_PIN3_RXD2 44   //Hardline UART RX pin 36 RXD0 (UART0 RX): GPIO 44. Looks to be defined as "RX" in arduino
//#define HeaderP1_4PIN_PIN4_GND     //Ground

//Header P2 is a JST1.25 8P IO port
#define HeaderP2_8PIN_PIN1  5
#define HeaderP2_8PIN_PIN2  6
#define HeaderP2_8PIN_PIN3  7
#define HeaderP2_8PIN_PIN4  15
#define HeaderP2_8PIN_PIN5  16
#define HeaderP2_8PIN_PIN6  46
#define HeaderP2_8PIN_PIN7  9
#define HeaderP2_8PIN_PIN8  14

// HEADER 3 and HEADER 4 share the same pins. Guessing its for i2c
//Header 3 is JST 1.25 4P
//#define Header3_4PIN_PIN1  GND
//#define Header3_4PIN_PIN2  3.3V
#define HeaderP3_4PIN_PIN3  17
#define HeaderP3_4PIN_PIN4  18

//Header 4 is HC1.0 4P
//#define Header4_4PIN_PIN1  GND
//#define Header4_4PIN_PIN2  3.3V
#define HeaderP4_4PIN_PIN3  17
#define HeaderP4_4PIN_PIN4  18

//Header P5 Is the 3.7v lipo battery terminal.
//Chip U2 is a IP5306 lithium battery charger chip which is supposed to charge at 4.2V. Input voltage 4.5-5.5V https://www.laskakit.cz/user/related_files/ip5306.pdf
//SW1 "Battery Switch button" goes to the KEY pin on U2. Short push will open SOC indicator LEDs and step-up converter.  Long push will close step-up convertor, SOC indicator LED and flashlight LED.
//Chip U4 NS4168 (2417Y2) appears to be the audio power amplifier. https://www.electrokit.com/upload/quick/12/91/662c_IP5306.pdf or  https://robu.in/wp-content/uploads/2021/08/1046862-NS4168_EN_datasheet.pdf
#endif