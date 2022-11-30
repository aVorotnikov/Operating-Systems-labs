#pragma once

enum GOAT_STATE
{
    ALIVE = 0,
    DEAD,
    ALMOST_DEAD,
};

struct Message
{
    unsigned short thrownNumber;
    GOAT_STATE goatState;
};
