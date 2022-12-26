#pragma once

#include "../connections/conn.h"
#include "../state.h"

#include <sys/types.h>
#include <memory>
#include <semaphore.h>

class Client {
private:
    pid_t hostPid;
    std::unique_ptr<Conn> conn;
    constexpr static int minRand = 1;
    constexpr static int minRandForAlive = 101;
    constexpr static int minRandForDead = 51;
    constexpr static int connTimeout = 5;
    sem_t* semRead;
    sem_t* semWrite;
    State state = State::ALIVE;

    bool OpenConnection();
    bool SendNumber();
    bool GetState();
public:
    Client(pid_t hostPid);
    void Run();
};
