#pragma once

enum GOAT_STATE
{
    ALIVE = 0,
    DEAD
};

enum GOAT_EVENT
{
    NONE = 0,
    DIED,
    HIDEN,
    REVIVED,
    THROW_REQUESTED,
    THROW_RECEIVED
};

struct GoatInfo
{
    short id;
    GOAT_STATE state;
    GOAT_EVENT lastEvent;
    ushort thrownNumber;

    GoatInfo(short id = -1)
    {
        this->id = id;
        state = GOAT_STATE::ALIVE;
        lastEvent = GOAT_EVENT::NONE;
        thrownNumber = 0;
    }
};

inline bool operator==(const GoatInfo &a, const GoatInfo &b)
{
    return a.id == b.id && a.state == b.state && a.lastEvent == b.lastEvent && a.thrownNumber == b.thrownNumber;
}