#include <Arduino.h>
#include <FastLED.h>
#include <Wifi.h>
// This is where we define variables we will use a lot, or need to spread among multiple files. These should be variables the user will change according to their preference.

#if LABLIGHTS // If the build flags include "LABLIGHTS" in platformio.ini


    #ifndef PROJECT_NAME
    #define PROJECT_NAME    "Lablights" // This is 
    #endif

    #define SWITCH_IP       IPAddress(192,168,2,14) // IP address of the switch you're targeting
    #define NUM_PORTS       48 // Number of ports on the switch
    #define POLL_DELAY      10000 // 10 seconds

    #define BRIGHTNESS      64 // default is 64
    #define NUM_LEDS        100 // adjust according to your strip size (May need more variables if using multiple strips)
    #define NUM_CHANNELS    1 // number of strips being used (currently has no importance)
    #define MAX_COMETS      10 // To prevent the device from saving more spots than necessary and possible causing delay or lag

    #define POWER_LIMIT_MW  (7.5 * 24 * 1000) // Expects at least a 24V 7.5A supply

    #define DATA_PIN        32 // Change to what pin you have your strip attached to (Add more variables for more strips ex: DATA_PIN2)
    #define LED_TYPE        WS2812B // Almost no clue what this means, the type leds being used or something
    #define COLOR_ORDER     GRB

    #define ARRAY_1        {1,2,3,4,5,6} // an array of ports you want to be checked for snmp data
    #define NUMOFPORTS1    6 // how many ports you defined above (I don't know how else to do this)
#endif