#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <condition_variable>
#include <chrono>
#include <mutex>

bool simulation_is_running();
void wait_if_paused();

class Semaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    Semaphore(int initial_count) : count(initial_count) {}

    bool wait() {
        std::unique_lock<std::mutex> lock(mtx);
        while (simulation_is_running()) {
            cv.wait_for(lock, std::chrono::milliseconds(50), [this]() {
                return count > 0 || !simulation_is_running();
            });
            if (!simulation_is_running()) return false;
            if (count > 0) {
                lock.unlock();
                wait_if_paused();
                lock.lock();
                if (!simulation_is_running()) return false;
                if (count > 0) { --count; return true; }
            }
        }
        return false;
    }

    void signal(bool notify_thread = true) {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        if (notify_thread) {
            cv.notify_one();
        }
    }

    void notify() {
        cv.notify_one();
    }

    void wake_all() { cv.notify_all(); }

    void reset(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        count = value;
        cv.notify_all();
    }

    int get_value() {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }
};

extern Semaphore empty_slots;
extern Semaphore full_slots;
extern Semaphore mutex_sem;

#endif
