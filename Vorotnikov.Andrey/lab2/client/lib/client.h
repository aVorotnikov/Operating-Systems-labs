#pragma once

#include "../../connections/conn.h"

#include <semaphore.h>

class Client
{
public:
    friend void SignalHandler(const int sig);

    static Client& GetRef();

    void SetConnection(std::shared_ptr<Connection> connection);
    bool Run();

private:
    static constexpr std::size_t timeoutOnSem_ = 10;
    static Client instance_;
    Client();

    bool SendNum();
    bool GetGhoatState();

    std::shared_ptr<Connection> connection_;
    bool needWork_;
    bool isAlive_;
    sem_t* semOnRead_;
    sem_t* semOnWrite_;
};