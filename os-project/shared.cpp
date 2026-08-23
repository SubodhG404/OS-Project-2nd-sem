#include "shared.h"

#include <chrono>
#include <condition_variable>

int buffer[BUFFER_SIZE];
int in_idx = 0;
int out_idx = 0;
std::atomic<int> item_counter{1};

std::string last_mover = "";

std::atomic<int> race_shared_counter{0};

std::mutex console_mutex;
std::mutex state_mutex;
std::string last_action = "Choose an execution mode, then press Start Simulation.";
std::atomic<bool> simulation_running{false};
std::atomic<bool> simulation_paused{false};

namespace {
std::mutex control_mutex;
std::condition_variable control_cv;
}

bool simulation_is_running() { return simulation_running.load(); }

void set_simulation_paused(bool paused) {
    simulation_paused.store(paused);
    control_cv.notify_all();
}

void wait_if_paused() {
    std::unique_lock<std::mutex> lock(control_mutex);
    control_cv.wait(lock, [] { return !simulation_running.load() || !simulation_paused.load(); });
}

bool wait_for_simulation_delay(int milliseconds) {
    constexpr int tick_ms = 20;
    int remaining = milliseconds;
    while (remaining > 0 && simulation_running.load()) {
        wait_if_paused();
        if (!simulation_running.load()) return false;
        const int slice = std::min(tick_ms, remaining);
        std::unique_lock<std::mutex> lock(control_mutex);
        control_cv.wait_for(lock, std::chrono::milliseconds(slice), [] {
            return !simulation_running.load() || simulation_paused.load();
        });
        if (!simulation_paused.load()) remaining -= slice;
    }
    return simulation_running.load();
}

void reset_shared_state() {
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        for (int& slot : buffer) slot = 0;
        in_idx = 0; out_idx = 0; item_counter.store(1); race_shared_counter.store(0);
    }
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        last_mover.clear();
        last_action = "Choose an execution mode, then press Start Simulation.";
    }
}
