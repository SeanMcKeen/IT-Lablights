#include "FastLED.h"

int calcSNMPPulses(unsigned long Avg) {
    if (Avg >= 1 && Avg <= 100000) {
        // Map Avg values in the range [1, 100000] to the range [1, 5]
        return round(1 + 4 * ((double)Avg / 100000));
    } else if (Avg > 100000 && Avg <= 1000000) {
        // Map Avg values in the range (100000, 1000000] to the range [6, 10]
        return round(6 + 4 * ((double)Avg - 100000) / 900000);
    }
    // Return a default value or handle other cases as necessary
    return 1;
}

CRGB calcPulseColor(unsigned long Avg) { // This is to make a reactive color depending on the packet size, in a CGRB strip, like mine, Green is Red and Red is Green
    if (Avg < 35000) {
        return CRGB::Red;
    }else if (Avg < 80000) {
        return CRGB::MediumVioletRed;
    }else if (Avg < 170000) {
        return CRGB::DarkBlue;
    }else if (Avg < 280000) {
        return CRGB::Blue;
    }else if (Avg < 420000) {
        return CRGB::Aquamarine;
    }else{
        return CRGB::Green;
    }
}

CRGB calcPulseColor2(unsigned long Avg) { // This is to have similar, but not exactly the same colors
    if (Avg < 35000) {
        return CRGB::PaleVioletRed;
    }else if (Avg < 80000) {
        return CRGB::Violet;
    }else if (Avg < 170000) {
        return CRGB::DarkCyan;
    }else if (Avg < 280000) {
        return CRGB::LightBlue;
    }else if (Avg < 420000) {
        return CRGB::GreenYellow;
    }else{
        return CRGB::LimeGreen;
    }
}