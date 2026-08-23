#ifndef SHARED_H
#define SHARED_H

#include <atomic>
#include <mutex>
#include <string>

#include "config.h"

extern int buffer[BUFFER_SIZE];
extern int in_idx;
extern int out_idx;
extern std::atomic<int> item_counter;
extern std::string last_mover;
extern std::atomic<int> race_shared_counter;
extern std::mutex console_mutex;
extern std::mutex state_mutex;
extern std::string last_action;
extern std::atomic<bool> simulation_running;
extern std::atomic<bool> simulation_paused;

bool simulation_is_running();
void set_simulation_paused(bool paused);
void wait_if_paused();
bool wait_for_simulation_delay(int milliseconds);
void reset_shared_state();

#endif
