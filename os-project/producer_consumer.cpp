#include "producer_consumer.h"
#include "config.h"
#include "semaphore.h"
#include "shared.h"
#include "visualization.h"

#include <chrono>
#include <string>
#include <thread>

void producer() {
    while (simulation_is_running()) {
        if (!wait_for_simulation_delay(PRODUCER_DELAY_MS)) break;

        int item = item_counter++;

        if (ENABLE_DEADLOCK_MODE) {
            // INCORRECT ORDERING: Lock mutex BEFORE checking for empty slots
            if (!mutex_sem.wait()) break;
            display_state("PRODUCER acquired Mutex (Checking Empty Slots...)");
            if (!empty_slots.wait()) { mutex_sem.signal(); break; } // Holds mutex when buffer fills: intentional deadlock.
        } else {
            // CORRECT ORDERING: Wait for empty slot first, then acquire mutex
            if (!empty_slots.wait()) break;
            if (!mutex_sem.wait()) { empty_slots.signal(); break; }
        }

        // Critical Section
        std::string action;
        {
            std::lock_guard<std::mutex> state_lock(state_mutex);
            buffer[in_idx] = item;
            action = "PRODUCER added Item " + std::to_string(item) + " at slot " + std::to_string(in_idx);
            in_idx = (in_idx + 1) % BUFFER_SIZE;
        }

        full_slots.signal(false); 

        display_state(action);

        full_slots.notify();
        mutex_sem.signal();

        // --------------------------------------------------------------
        // RACE CONDITION DEMO (mode 5 only): deliberately unsynchronized
        // read-modify-write on race_shared_counter. No mutex/atomic is
        // used here on purpose, and a small delay is inserted between the
        // read and the write to widen the window for the consumer thread
        // to interleave and cause a lost update. This is fully separate
        // from the buffer/semaphore logic above, which is unaffected.
        // --------------------------------------------------------------
        if (ENABLE_RACE_CONDITION_MODE) {
            int temp = race_shared_counter.load();
            if (!wait_for_simulation_delay(50)) break;
            temp = temp + 1;
            race_shared_counter.store(temp); // deliberately non-atomic read-modify-write: lost updates are observable safely
        }
    }
}

void consumer() {
    while (simulation_is_running()) {
        if (!wait_for_simulation_delay(CONSUMER_DELAY_MS)) break;

        // Synchronization (Always correct ordering)
        if (!full_slots.wait()) break;

        // In Deadlock mode, consumer will hang here forever trying to acquire mutex_sem held by producer
        if (!mutex_sem.wait()) { full_slots.signal(); break; }

        // Critical Section
        std::string action;
        {
            std::lock_guard<std::mutex> state_lock(state_mutex);
            int item = buffer[out_idx];
            buffer[out_idx] = 0;
            action = "CONSUMER took Item " + std::to_string(item) + " from slot " + std::to_string(out_idx);
            out_idx = (out_idx + 1) % BUFFER_SIZE;
        }

        empty_slots.signal(false);

        display_state(action);

        empty_slots.notify();
        mutex_sem.signal();

        // --------------------------------------------------------------
        // RACE CONDITION DEMO (mode 5 only): same unsynchronized access
        // pattern as the producer, so both threads race on the same
        // plain int with no locking. Buffer/semaphore logic above is
        // untouched and continues to work correctly.
        // --------------------------------------------------------------
        if (ENABLE_RACE_CONDITION_MODE) {
            int temp = race_shared_counter.load();
            if (!wait_for_simulation_delay(50)) break;
            temp = temp - 1;
            race_shared_counter.store(temp); // deliberately non-atomic read-modify-write: lost updates are observable safely
        }
    }
}
