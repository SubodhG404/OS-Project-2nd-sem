#include "config.h"
#include <iostream>

int PRODUCER_DELAY_MS = 2000;
int CONSUMER_DELAY_MS = 2000;
bool ENABLE_DEADLOCK_MODE = false;
bool ENABLE_RACE_CONDITION_MODE = false;

void configure_rates(int mode_choice) {
    ENABLE_DEADLOCK_MODE = false;
    ENABLE_RACE_CONDITION_MODE = false;
    switch (mode_choice) {
        case 1:
            PRODUCER_DELAY_MS = 2000; 
            CONSUMER_DELAY_MS = 5000; 
            break;
        case 2:
            PRODUCER_DELAY_MS = 6000; 
            CONSUMER_DELAY_MS = 3000; 
            break;
        case 3:
            PRODUCER_DELAY_MS = 3000; 
            CONSUMER_DELAY_MS = 3000; 
            break;
        case 4:
            // Fast producer, slow consumer to fill buffer quickly and trigger deadlock
            PRODUCER_DELAY_MS = 500;
            CONSUMER_DELAY_MS = 3000;
            ENABLE_DEADLOCK_MODE = true;
            break;
        case 5:
            // Fast, roughly equal delays so producer and consumer overlap frequently,
            // maximizing the chance of interleaved (unsynchronized) access to race_shared_counter
            PRODUCER_DELAY_MS = 400;
            CONSUMER_DELAY_MS = 400;
            ENABLE_RACE_CONDITION_MODE = true;
            break;
        default:
            std::cout << "Invalid mode choice! Please enter a value between 1 and 5. Defaulting to Mode 3 (Balanced).\n";
            PRODUCER_DELAY_MS = 3000;
            CONSUMER_DELAY_MS = 3000;
            break;
    }
}
