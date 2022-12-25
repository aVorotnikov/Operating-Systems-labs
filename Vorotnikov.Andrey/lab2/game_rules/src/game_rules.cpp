#include "../game_rules.h"

#include <cstdlib>

namespace game_rules
{

namespace
{

int GetRandom(Interval range)
{
    return (static_cast<double>(rand()) / RAND_MAX) * (range.max - range.min) + range.min;
}

}

int GetRandomForWolf()
{
    return GetRandom(wolfRange);
}

int GetRandomForAliveGhoat()
{
    return GetRandom(aliveGhoatRange);
}

int GetRandomForDeadGhoat()
{
    return GetRandom(deadGhoatRange);
}

int GetRandomForGhoat(bool isAlive)
{
    if (isAlive)
        return GetRandomForAliveGhoat();
    return GetRandomForDeadGhoat();
}

bool CheckStateForAliveGoat(int wolf, int goat, int ghoatNumber)
{
    if (std::abs(wolf - goat) > 70 / ghoatNumber)
        return false;
    return true;
}

bool CheckStateForDeadGoat(int wolf, int goat, int ghoatNumber)
{
    if (std::abs(wolf - goat) > 20 / ghoatNumber)
        return false;
    return true;
}

}
