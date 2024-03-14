#include <TFT_eSPI.h> // Include the TFT_eSPI library for the display
#include "lilygoPINOUT.h"

TFT_eSPI tft; // Create a TFT_eSPI object

void setupDisplay() {
    pinMode(PIN_POWER_ON, OUTPUT);  //enables the LCD and to run on battery
    digitalWrite(PIN_POWER_ON, HIGH);
    
    tft.init(); // Initialize the display
    tft.setRotation(1); // Set display rotation if necessary
}

void refreshScreen() {
    tft.fillScreen(TFT_BLACK); // Clear the screen
    tft.setCursor(0, 0, 2); // Set cursor position
    tft.setTextColor(TFT_WHITE); // Set text color
    tft.setTextSize(1); // Set text size
    tft.println("Hello, World!"); // Print text to the display
}

