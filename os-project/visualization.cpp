#include "visualization.h"

#include "config.h"
#include "simulation_controller.h"
#include "shared.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <sstream>
#include <mutex>

namespace {
constexpr unsigned WIDTH = 1180, HEIGHT = 760;
struct Mode { const char* title; const char* detail; };
const std::array<Mode, 5> modes{{
    {"Producer Faster", "2s produce / 5s consume"}, {"Consumer Faster", "6s produce / 3s consume"},
    {"Balanced Rates", "3s produce / 3s consume"}, {"Deadlock Demo", "Wrong lock ordering"},
    {"Race Condition", "Unsynchronized counter"}
}};

void box(sf::RenderWindow& w, sf::Vector2f p, sf::Vector2f s, sf::Color fill,
         sf::Color outline = sf::Color::Transparent, float thickness = 0) {
    sf::RectangleShape r(s); r.setPosition(p); r.setFillColor(fill); r.setOutlineColor(outline); r.setOutlineThickness(thickness); w.draw(r);
}
void text(sf::RenderWindow& w, const sf::Font& f, const std::string& value, unsigned size, sf::Vector2f p, sf::Color c = sf::Color::White) {
    sf::Text t(f, value, size); t.setPosition(p); t.setFillColor(c); w.draw(t);
}
bool hit(sf::Vector2f p, sf::FloatRect r) { return r.contains(p); }
std::string font_path() {
    for (const char* path : {"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf", "DejaVuSans.ttf"}) {
        sf::Font f; if (f.openFromFile(path)) return path;
    }
    return {};
}
}

// Worker threads publish state; all window drawing remains on the SFML thread.
void display_state(const std::string& action) {
    std::lock_guard<std::mutex> lock(console_mutex);
    last_action = action;
    if (action.find("PRODUCER") != std::string::npos) last_mover = "P";
    else if (action.find("CONSUMER") != std::string::npos) last_mover = "C";
}

void run_visualization() {
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Concurrency Lab");
    window.setFramerateLimit(60);
    sf::Font font; const auto path = font_path();
    if (path.empty() || !font.openFromFile(path)) return;

    int selected = 3;
    SimulationController simulation;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) { simulation.stop(); window.close(); }
            if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (click->button != sf::Mouse::Button::Left) continue;
                sf::Vector2f mouse(click->position);
                if (!simulation.is_running()) {
                    for (int i = 0; i < 5; ++i)
                        if (hit(mouse, {{35.f + i * 225.f, 103.f}, {205.f, 75.f}})) selected = i + 1;
                    if (hit(mouse, {{350, 655}, {160, 52}})) simulation.start(selected);
                } else {
                    if (hit(mouse, {{530, 655}, {160, 52}})) {
                        simulation.toggle_pause();
                    }
                    if (hit(mouse, {{710, 655}, {160, 52}})) simulation.stop();
                }
            }
        }

        const SimulationSnapshot snapshot = simulation.snapshot();

        window.clear(sf::Color(24, 26, 31));
        box(window, {0, 0}, {float(WIDTH), 82}, sf::Color(31, 34, 41));
        box(window, {0, 80}, {float(WIDTH), 2}, sf::Color(79, 126, 247));
        text(window, font, "Concurrency Lab", 27, {35, 16}, sf::Color(241, 243, 245));
        text(window, font, "Producer–consumer synchronization", 15, {38, 50}, sf::Color(169, 176, 188));
        const bool paused = snapshot.paused;
        text(window, font, !snapshot.running ? "Ready" : (paused ? "Paused" : "Running"), 17, {1040, 30}, !snapshot.running ? sf::Color(221, 169, 66) : (paused ? sf::Color(221, 169, 66) : sf::Color(91, 193, 129)));
        for (int i = 0; i < 5; ++i) {
            bool chosen = selected == i + 1; float x = 35.f + i * 225.f;
            box(window, {x, 103}, {205, 75}, chosen ? sf::Color(42, 62, 104) : sf::Color(31, 34, 41), chosen ? sf::Color(100, 143, 242) : sf::Color(57, 62, 73), 2);
            text(window, font, modes[i].title, 15, {x + 13, 117}, sf::Color(238, 240, 244)); text(window, font, modes[i].detail, 12, {x + 13, 144}, sf::Color(174, 180, 190));
        }
        box(window, {35, 205}, {1110, 57}, sf::Color(31, 34, 41));
        text(window, font, "Last activity", 13, {52, 219}, sf::Color(154, 161, 173)); text(window, font, snapshot.last_action, 17, {190, 216}, sf::Color(235, 237, 241));
        sf::Color pc = snapshot.last_mover == "P" ? sf::Color(222, 176, 74) : sf::Color(79, 160, 181);
        sf::Color cc = snapshot.last_mover == "C" ? sf::Color(222, 176, 74) : sf::Color(143, 117, 184);
        box(window, {65, 365}, {190, 90}, pc); box(window, {925, 365}, {190, 90}, cc);
        text(window, font, "Producer", 22, {93, 394}, sf::Color(24, 26, 31)); text(window, font, "Consumer", 22, {952, 394}, sf::Color(24, 26, 31));
        text(window, font, "→", 42, {290, 383}, snapshot.last_mover == "P" ? sf::Color(91, 193, 129) : sf::Color(96, 102, 114));
        text(window, font, "→", 42, {840, 383}, snapshot.last_mover == "C" ? sf::Color(91, 193, 129) : sf::Color(96, 102, 114));
        text(window, font, "Bounded buffer", 18, {480, 294}, sf::Color(221, 224, 229));
        for (int i = 0; i < BUFFER_SIZE; ++i) {
            float x = 360.f + i * 94.f; bool occupied = snapshot.buffer[i] != 0;
            box(window, {x, 355}, {76, 110}, occupied ? sf::Color(36, 82, 58) : sf::Color(48, 53, 63), i == snapshot.in_index ? sf::Color(222, 176, 74) : sf::Color(78, 84, 96), 3);
            text(window, font, "Slot " + std::to_string(i + 1), 12, {x + 11, 367}, sf::Color(190, 196, 205));
            text(window, font, occupied ? "I" + std::to_string(snapshot.buffer[i]) : "—", 24, {x + 20, 401}, occupied ? sf::Color(156, 219, 176) : sf::Color(148, 154, 165));
            if (i == snapshot.in_index) text(window, font, "in", 12, {x + 25, 474}, sf::Color(222, 176, 74));
            if (i == snapshot.out_index) text(window, font, "out", 12, {x + 18, 493}, sf::Color(104, 181, 202));
        }
        box(window, {35, 540}, {1110, 72}, sf::Color(31, 34, 41));
        text(window, font, "Available: " + std::to_string(snapshot.empty_slots), 17, {65, 557}, sf::Color(121, 180, 201));
        text(window, font, "Queued: " + std::to_string(snapshot.full_slots), 17, {290, 557}, sf::Color(142, 204, 161));
        text(window, font, "Producer interval: " + std::to_string(snapshot.producer_delay_ms / 1000.0) + "s", 17, {480, 557}, sf::Color(215, 218, 224));
        text(window, font, "Consumer interval: " + std::to_string(snapshot.consumer_delay_ms / 1000.0) + "s", 17, {715, 557}, sf::Color(215, 218, 224));
        if (snapshot.deadlock_mode) text(window, font, "WARNING: DEADLOCK DEMO ACTIVE", 15, {65, 583}, sf::Color(252, 165, 165));
        if (snapshot.race_mode) text(window, font, "RACE COUNTER (unsynchronized): " + std::to_string(snapshot.race_counter), 15, {65, 583}, sf::Color(252, 165, 165));
        box(window, {350, 655}, {160, 52}, snapshot.running ? sf::Color(71, 85, 105) : sf::Color(22, 163, 74));
        text(window, font, "Start", 18, {397, 670});
        box(window, {530, 655}, {160, 52}, snapshot.running ? sf::Color(202, 138, 4) : sf::Color(71, 85, 105));
        text(window, font, paused ? "Resume" : "Pause", 18, {paused ? 567.f : 577.f, 670});
        box(window, {710, 655}, {160, 52}, snapshot.running ? sf::Color(185, 28, 28) : sf::Color(71, 85, 105));
        text(window, font, "Stop & reset", 16, {730, 671});
        window.display();
    }
    simulation.stop();
}
