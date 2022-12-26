#pragma once

#include <string>
#include <atomic>
#include <memory>
#include "../utils/threadsafe_vector.hpp"

enum CONNECT_STATUS {
    CONNECTION,
    DISCONNECTION,
    LOAD,
};

enum GOAT_STATE {
    DEAD,
    ALIVE
};

struct Goat {
    std::atomic<int> number;
    std::atomic<GOAT_STATE> state;
    std::atomic<CONNECT_STATUS> clientStatus;
};

struct GameState {
    std::atomic<int> wolfNumber;
    std::atomic<int> deadNumber;
    std::atomic<int> aliveNumber;
    Vec<std::shared_ptr<Goat>> goats;
    std::atomic<int> time;
};