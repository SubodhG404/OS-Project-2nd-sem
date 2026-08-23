#include "simulation_controller.h"

#include "producer_consumer.h"
#include "semaphore.h"
#include "shared.h"
#include "visualization.h"

SimulationController::~SimulationController() { stop(); }

void SimulationController::start(int mode) {
    if (running_) return;

    reset_shared_state();
    empty_slots.reset(BUFFER_SIZE);
    full_slots.reset(0);
    mutex_sem.reset(1);
    configure_rates(mode);
    simulation_running.store(true);
    set_simulation_paused(false);
    display_state("System initialized - simulation is running");
    running_ = true;
    producer_thread_ = std::thread(producer);
    consumer_thread_ = std::thread(consumer);
}

void SimulationController::toggle_pause() {
    if (!running_) return;
    const bool pausing = !simulation_paused.load();
    set_simulation_paused(pausing);
    display_state(pausing ? "Simulation paused" : "Simulation resumed");
}

void SimulationController::stop() {
    if (!running_) return;

    simulation_running.store(false);
    set_simulation_paused(false);
    empty_slots.wake_all();
    full_slots.wake_all();
    mutex_sem.wake_all();
    if (producer_thread_.joinable()) producer_thread_.join();
    if (consumer_thread_.joinable()) consumer_thread_.join();
    empty_slots.reset(BUFFER_SIZE);
    full_slots.reset(0);
    mutex_sem.reset(1);
    running_ = false;
    reset_shared_state();
    display_state("Simulation stopped and reset. Select a mode to start again.");
}

bool SimulationController::is_running() const { return running_; }

SimulationSnapshot SimulationController::snapshot() const {
    SimulationSnapshot result;
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        for (int i = 0; i < BUFFER_SIZE; ++i) result.buffer[i] = buffer[i];
        result.in_index = in_idx;
        result.out_index = out_idx;
    }
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        result.last_action = last_action;
        result.last_mover = last_mover;
    }
    result.empty_slots = empty_slots.get_value();
    result.full_slots = full_slots.get_value();
    result.race_counter = race_shared_counter.load();
    result.producer_delay_ms = PRODUCER_DELAY_MS;
    result.consumer_delay_ms = CONSUMER_DELAY_MS;
    result.running = running_;
    result.paused = simulation_paused.load();
    result.deadlock_mode = ENABLE_DEADLOCK_MODE;
    result.race_mode = ENABLE_RACE_CONDITION_MODE;
    return result;
}
