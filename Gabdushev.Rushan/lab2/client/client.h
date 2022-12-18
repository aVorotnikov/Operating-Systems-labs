#pragma once

#include <semaphore.h>
#include <chrono>
#include <csignal>

#include "connections/conn.h"
#include "utils/goat_info.h"

class Client
{
private:
    bool needToStop;
    bool throwRequested;
    Connection *conn = nullptr;
    sem_t *semIn = SEM_FAILED;
    sem_t *semOut = SEM_FAILED;
    pid_t hostPid;
    short goatId;
    Message lastMessage;
    Message replyMessage;

public:
    static Client &getInstance();
    bool init(pid_t hostPid, short goatId, int randomSeed);
    void run();

private:
    Client() = default;
    Client(Client &w) = delete;
    Client &operator=(const Client &w) = delete;
    void stop();
    void closeConnection();
    bool readMessage();
    bool handleMessage();
    ushort throwNumber();
    bool sendMessage();
    static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);
};