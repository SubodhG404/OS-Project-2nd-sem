#include "semaphore.h"
#include "config.h"

Semaphore empty_slots(BUFFER_SIZE);
Semaphore full_slots(0);
Semaphore mutex_sem(1);
