#include <mathhandler.h>

int calcSNMPPulses(int Avg) { // Where we determine data to pulse ratio, an average of 34000 packets will send 3 pulses, while 36000 will send 5, this can be changed however desired
    if (Avg < 1000) {
        return 1;
    }else if (Avg < 10000) {
        return 2;
    }else if (Avg < 35000) {
        return 3;
    }else if (Avg < 100000) {
        return 5;
    }else if (Avg < 200000) {
        return 6;
    }else if (Avg < 300000) {
        return 7;
    }else if (Avg < 400000) {
        return 8;
    }else if (Avg < 500000) {
        return 9;
    }else{
        return 10;
    }
}

CRGB calcPulseColor(int Avg) { // This is to make a reactive color depending on the packet size, in a CGRB strip, like mine, Green is Red and Red is Green
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

CRGB calcPulseColor2(int Avg) { // This is to have similar, but not exactly the same colors
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