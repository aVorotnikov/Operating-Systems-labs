#pragma once

#include <cstdlib>
#include <string>

namespace Configuration
{
    static const std::string HOST_SEMAPHORE_NAME = "wolf_semaphore";
    static const std::string CLIENT_SEMAPHORE_NAME = "goat_semaphore";

    static const size_t TIME_OUT = 5;
    static const size_t ROUND_TIME_IN_MS = 3000;

    namespace Wolf  
    {
        static const size_t MIN_NUMBER = 1;
        static const size_t MAX_NUMBER = 100;
    };

    namespace Goat
    {
        static const size_t MIN_NUMBER = 1;
        static const size_t MAX_ALIVE_NUMBER = 100;
        static const size_t MAX_ALMOST_DEATH_NUMBER = 50;
        static const size_t ALMOST_DEAD_GOAT_VALIDATION = 20;
        static const size_t ALIVE_GOAT_VALIDATION = 70;
    };
};