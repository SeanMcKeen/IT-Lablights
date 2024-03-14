#include <Arduino.h>
#include <FastLED.h>
// This is where we define variables we will use a lot, or need to spread among multiple files. These should be variables the user will change according to their preference.

#if LABLIGHTS // If the build flags include "LABLIGHTS" in platformio.ini


    #ifndef PROJECT_NAME
    #define PROJECT_NAME    "Lablights" // This is the main project, but in case we want to eventually have more.
    #endif
    
    #define DEBUG           0
    
    #define POLL_DELAY      10000 // 10 seconds

    #define BRIGHTNESS      100 // default is 64; MAX: 255
    #define NUM_CHANNELS    4 // number of strips being used
    #define MAX_COMETS      100 // To prevent the device from saving more spots than necessary and possibly causing delay or lag
    #define SPEED           20 // Interval in milliseconds (How fast the pulses travel, decreasing this value increases the speeds) default: 20

    #define POWER_LIMIT_MW  (7.5 * 24 * 1000) // Expects at least a 24V 7.5A supply

 // Change to what pin(s) you have your strip(s) attached to (Add more variables for more strips ex: DATA_PIN2)
 // adjust NUM_LEDS according to your strip sizes

    #define DATA_PIN1        10
    #define NUM_LEDS         31 

    #define DATA_PIN2        13
    #define NUM_LEDS2        31 

    #define DATA_PIN3        11
    #define NUM_LEDS3        31 

    #define DATA_PIN4        12
    #define NUM_LEDS4        31 

    #define LED_TYPE        WS2812B // Almost no clue what this means, the type of leds being used or something
    #define COLOR_ORDER     RGB // Currently affects nothing

    #define Strip1Ports     {1,2} // These are for demonstration purposes only, they do not affect anything as the data is simulated.
    #define Strip2Ports     {3,4}
    #define Strip3Ports     {5,6}
    #define Strip4Ports     {7,8}
#endif
