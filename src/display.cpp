#include <TFT_eSPI.h> // Include the TFT_eSPI library for the display

TFT_eSPI tft; // Create a TFT_eSPI object

#define PIN_POWER_ON 15  // LCD and battery Power Enable
#define PIN_LCD_BL 38    // BackLight enable pin (see Dimming.txt)

void setupDisplay() {
    pinMode(PIN_POWER_ON, OUTPUT);  //enables the LCD and to run on battery
    pinMode(PIN_LCD_BL, OUTPUT);    // BackLight enable pin
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);

    digitalWrite(PIN_POWER_ON, HIGH);
    digitalWrite(PIN_LCD_BL, HIGH);
    
    tft.begin(); // Initialize the display
    tft.setRotation(1); // Set display rotation if necessary
}

void refreshScreen() {
    tft.fillScreen(TFT_BLACK); // Clear the screen
    tft.setCursor(0, 0); // Set cursor position
    tft.setTextColor(TFT_WHITE); // Set text color
    tft.setTextSize(2); // Set text size
    tft.println("Hello, World!"); // Print text to the display
}

