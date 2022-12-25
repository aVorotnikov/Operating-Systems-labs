#pragma once

#include "../../connections/conn.h"

class Client
{
public:
    friend void SignalHandler(const int sig);

    static Client& GetRef();

    void SetConnection(std::shared_ptr<Connection> connection);
    bool Run();

private:
    static Client instance_;
    Client();

    std::shared_ptr<Connection> connection_;
    bool needWork_;
};