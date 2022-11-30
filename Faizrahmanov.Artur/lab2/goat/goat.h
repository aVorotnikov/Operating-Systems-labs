#pragma once

#include "connections/conn.h"
#include <semaphore.h>

class Goat
{
private:
    bool isRun = false;
    pid_t hostPid = 0;
    sem_t *hostSemaphore;
    sem_t *clientSemaphore;
    std::unique_ptr<Connection> conn;

public:
    static Goat &getInstance();
    ~Goat();

    bool init(const pid_t &hostPid);
    void run();

private:
    Goat();
    Goat(Goat &w) = delete;
    Goat &operator=(const Goat &w) = delete;

    static void signalHandler(int signal);
    bool getWolfMessage(Message &msg);
    size_t getGoatNumber(GOAT_STATE goatState);
    bool sendGoatMessage(const Message &msg);
    bool stopClient();
};