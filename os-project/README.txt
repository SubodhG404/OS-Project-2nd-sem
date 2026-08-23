FILES
-----
main.cpp                 Main program and startup
config.h / config.cpp    Configuration and execution modes
shared.h / shared.cpp    Shared buffer/state variables
semaphore.h / semaphore.cpp
                         Custom semaphore and semaphore instances
visualization.h / visualization.cpp
                         SFML window, input handling, and dashboard rendering
producer_consumer.h / producer_consumer.cpp
                         Producer and consumer thread functions
simulation_controller.h / simulation_controller.cpp
                         Simulation lifecycle, thread ownership, and snapshots

GUI CONTROLS
------------
Select a mode in the SFML window and press **Start Simulation**. The dashboard
shows buffer slots, IN/OUT pointers, semaphore counts, rates, and warnings for
the deadlock and race-condition demonstrations.

- **Pause / Resume** freezes and continues simulation time without losing state.
- **Stop / Reset** cancels both workers, waits for them to exit, resets all
  semaphores and buffer slots, and allows another mode to be selected.
- Closing the window performs the same clean cancellation before exiting.

DEPENDENCY
----------
SFML 3 graphics library (Debian/Ubuntu: `sudo apt install libsfml-dev`).

COMPILE
-------
make

The Makefile uses CMake so it can link the bundled static SFML 3 libraries and
their platform dependencies correctly.

RUN
---
./build/producer_consumer

If an older `./producer_consumer` process is already open, close it and start
the current build from `./build/producer_consumer`.

The original simulation modes and synchronization behavior are preserved, while
the GUI owns presentation and `SimulationController` owns lifecycle management.
