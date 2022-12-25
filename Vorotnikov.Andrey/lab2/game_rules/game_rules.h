#pragma once

namespace game_rules
{

struct Interval
{
    int min;
    int max;
};

static constexpr Interval wolfRange = {1, 100};
static constexpr Interval aliveGhoatRange = {1, 100};
static constexpr Interval deadGhoatRange = {1, 50};

int GetRandomForWolf();
int GetRandomForAliveGhoat();
int GetRandomForDeadGhoat();
int GetRandomForGhoat(bool isAlive);

bool CheckStateForAliveGoat(int wolf, int goat, int aliveNumber);
bool CheckStateForDeadGoat(int wolf, int goat, int aliveNumber);

}
