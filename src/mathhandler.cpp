#include "FastLED.h"
#include <cstdlib>   // Include the <cstdlib> header for the rand() function

int calcSNMPPulses(unsigned long Avg) {
    if (Avg >= 1 && Avg <= 100000) {
        // Map Avg values in the range [1, 100000] to the range [1, 4]
        return round(1 + 3 * ((double)Avg / 100000));
    } else if (Avg > 100000 && Avg <= 1000000) {
        // Map Avg values in the range (100000, 1000000] to the range [5, 7]
        return round(5 + 2 * ((double)Avg - 100000) / 900000);
    } else if (Avg > 1000000 && Avg <= 10000000) {
        // Map Avg values in the range (1000000, 10000000] to the range [8, 10]
        return round(8 + 2 * ((double)Avg - 1000000) / 9000000);
    } else if (Avg > 10000000) {
        // Handle values greater than 10,000,000, return a default value or handle as necessary
        return 10;
    }
    return 1;
}

CRGB calcPulseColor(unsigned long Avg) { // This is to make a reactive color depending on the packet size, in a CGRB strip, like mine, Green is Red and Red is Green
    if (Avg < 35000) {
        return CRGB::Green;
    }else if (Avg < 80000) {
        return CRGB::Aquamarine;
    }else if (Avg < 170000) {
        return CRGB::Blue;
    }else if (Avg < 280000) {
        return CRGB::DarkBlue;
    }else if (Avg < 420000) {
        return CRGB::MediumVioletRed;
    }else{
        return CRGB::Red;
    }
}

CRGB calcPulseColor2(unsigned long Avg) { // This is to have similar, but not exactly the same colors
    if (Avg < 35000) {
        return CRGB(5420608);
    }else if (Avg < 80000) {
        return CRGB(12320767);
    }else if (Avg < 170000) {
        return CRGB(7948287);
    }else if (Avg < 280000) {
        return CRGB(5649602);
    }else if (Avg < 420000) {
        return CRGB(16735930);
    }else{
        return CRGB(16736315);
    }
}

int randomZeroOrOne() {
    return rand() % 2; // Returns either 0 or 1
}