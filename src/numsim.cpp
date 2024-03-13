#include <iostream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include "globals.h"

const int NUM_STRIPS = NUM_CHANNELS;
int arrTotals[NUM_STRIPS][2] = {{0}};

void initSim() {
    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

int getRandom(int min, int max) {
    // Generate
    int random_number = std::rand() % (max - min + 1) + min;
    return random_number;
}

void normalTraffic(int strip) {
    arrTotals[strip - 1][0] = getRandom(15000, 75000);
    arrTotals[strip - 1][1] = getRandom(15000, 75000);
}

void smallSpike(int strip) {
    arrTotals[strip - 1][0] = getRandom(100000, 200000);
    arrTotals[strip - 1][1] = getRandom(100000, 200000);
}

void mediumSpike(int strip) {
    arrTotals[strip - 1][0] = getRandom(250000, 700000);
    arrTotals[strip - 1][1] = getRandom(250000, 700000);
}

void largeSpike(int strip) {
    arrTotals[strip - 1][0] = getRandom(800000, 1000000);
    arrTotals[strip - 1][1] = getRandom(800000, 1000000);
}

void simulateTraffic() {
    int rNum = getRandom(0, 100);
    int amountOfStrips = getRandom(1, NUM_STRIPS);
    bool strips[4];
    
    for (int s = 0; s < NUM_STRIPS; s++){
        arrTotals[s][0] = 0;
        arrTotals[s][1] = 0;
    }
    int stripChosen;
    if (rNum <= 25) {
        for (int i = 0; i < amountOfStrips; i++) {
            stripChosen = getRandom(1, NUM_STRIPS);
            strips[stripChosen-1] = true;
            normalTraffic(stripChosen);
        }
    } else if (rNum <= 50) {
        for (int i = 0; i < amountOfStrips; i++) {
            stripChosen = getRandom(1, NUM_STRIPS);
            strips[stripChosen-1] = true;
            smallSpike(stripChosen);
        }
    } else if (rNum <= 75) {
        for (int i = 0; i < amountOfStrips; i++) {
            stripChosen = getRandom(1, NUM_STRIPS);
            strips[stripChosen-1] = true;
            mediumSpike(stripChosen);
        }
    } else {
        for (int i = 0; i < amountOfStrips; i++) {
            stripChosen = getRandom(1, NUM_STRIPS);
            strips[stripChosen-1] = true;
            largeSpike(stripChosen);
        }
    }
    for (int s = 0; s < NUM_STRIPS; s++){
        if (!strips[s]){
            normalTraffic(s);
        }
    }
}