#include "../client.h"

#include <csignal>

extern Client Client::instance_;

void SignalHandler(const int sig)
{
    auto& instance = Client::instance_;
    switch(sig)
    {
    case SIGTERM:
        instance.needWork_ = false;
        break;
    default:
        break;
    }
}

Client& Client::GetRef()
{
    return instance_;
}

Client::Client() : connection_(nullptr), needWork_(false)
{
}

void Client::SetConnection(std::shared_ptr<Connection> connection)
{
    connection_ = connection;
}

bool Client::Run()
{
    if (!connection_)
        return false;
    needWork_ = true;

    while (needWork_)
    {
        // work
    }

    return true;
}
