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
    #define NUM_CHANNELS    4 // number of strips being used (currently has no importance)
    #define MAX_COMETS      10 // To prevent the device from saving more spots than necessary and possible causing delay or lag

    #define POWER_LIMIT_MW  (7.5 * 24 * 1000) // Expects at least a 24V 7.5A supply

 // Change to what pin(s) you have your strip(s) attached to (Add more variables for more strips ex: DATA_PIN2)
 // adjust NUM_LEDS according to your strip sizes

    #define DATA_PIN1        10
    #define NUM_LEDS         30 

    #define DATA_PIN2        13
    #define NUM_LEDS2        30 

    #define DATA_PIN3        11
    #define NUM_LEDS3        30 

    #define DATA_PIN4        12
    #define NUM_LEDS4        30 

    #define LED_TYPE        WS2812B // Almost no clue what this means, the type leds being used or something
    #define COLOR_ORDER     GRB

    #define ARRAY_1        {1,2,3,4,5,6} // an array of ports you want to be checked for snmp data
    #define NUMOFPORTS1    sizeof(ARRAY_1) // how many ports you defined above
#endif