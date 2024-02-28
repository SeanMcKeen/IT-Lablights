#include <Arduino.h>
#include <FastLED.h>
#include <Wifi.h>
// This is where we define variables we will use a lot, or need to spread among multiple files. These should be variables the user will change according to their preference.

#if LABLIGHTS // If the build flags include "LABLIGHTS" in platformio.ini


    #ifndef PROJECT_NAME
    #define PROJECT_NAME    "Lablights" // This is the main project, but in case we want to eventually have more.
    #endif
    
    // Switch & SNMP Settings
    #define SWITCH_IP       IPAddress(192,168,2,14) // IP address of the switch you're targeting
    #define IN_OCTET        ".1.3.6.1.2.1.2.2.1.10." // Turns out these aren't the same for everyone, make sure you find this out.
    #define OUT_OCTET       ".1.3.6.1.2.1.2.2.1.16."
    #define SNMP_COMM       "public" // Community value of snmp, default: "public"
    #define NUM_PORTS       48 // Number of ports on the switch
    #define POLL_DELAY      10000 // Default: 10000 or 10 seconds
    #define DATA_CAP        500000 // Cap out the data so we don't get huge numbers like 3 billion if we dont want them.
    #define SNMPDEBUG       1

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

    #define ARRAY_1         {1,2} // an array of ports you want to be checked for snmp data
    #define NUMOFPORTS1     2 // how many ports you defined above, idk how else to do this

    #define ARRAY_2         {3,4} // leave as {} if not being used.
    #define NUMOFPORTS2     2

    #define ARRAY_3         {5,6}
    #define NUMOFPORTS3     2

    #define ARRAY_4         {7,8}
    #define NUMOFPORTS4     2
#endif