#ifndef SIMULATION_CONTROLLER_H
#define SIMULATION_CONTROLLER_H

#include "config.h"

#include <array>
#include <string>
#include <thread>

// A read-only frame of the simulator. The UI never reads shared state directly.
struct SimulationSnapshot {
    std::array<int, BUFFER_SIZE> buffer{};
    int in_index = 0;
    int out_index = 0;
    int empty_slots = BUFFER_SIZE;
    int full_slots = 0;
    int race_counter = 0;
    int producer_delay_ms = 0;
    int consumer_delay_ms = 0;
    bool running = false;
    bool paused = false;
    bool deadlock_mode = false;
    bool race_mode = false;
    std::string last_action;
    std::string last_mover;
};

class SimulationController {
public:
    ~SimulationController();

    void start(int mode);
    void toggle_pause();
    void stop();
    bool is_running() const;
    SimulationSnapshot snapshot() const;

private:
    bool running_ = false;
    std::thread producer_thread_;
    std::thread consumer_thread_;
};

#endif
