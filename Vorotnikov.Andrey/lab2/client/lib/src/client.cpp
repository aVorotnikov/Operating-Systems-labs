#include "../client.h"
#include "../../../game_rules/game_rules.h"
#include "../../../utils/raw.h"

#include <csignal>
#include <sys/syslog.h>

#include <string>

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

Client::Client() : connection_(nullptr), needWork_(false), isAlive_(true), semOnRead_(nullptr), semOnWrite_(nullptr)
{
}

void Client::SetConnection(std::shared_ptr<Connection> connection)
{
    connection_ = connection;
    syslog(LOG_INFO, "Connection set");
}

bool Client::SendNum()
{
    if (!utils::SendRaw(connection_, game_rules::GetRandomForGhoat(isAlive_)))
    {
        syslog(LOG_ERR, "Failed to connect with host while sending");
        return false;
    }
    sem_post(semOnWrite_);
    return true;
}

bool Client::GetGhoatState()
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += timeoutOnSem_;

    int waiting = sem_timedwait(semOnRead_, &t);
    if (-1 == waiting)
    {
        syslog(LOG_ERR, "Failed to wait server response");
        return false;
    }

    if (!utils::GetRaw(connection_, isAlive_))
    {
        syslog(LOG_ERR, "Failed to connect with host while getting");
        return false;
    }
    return true;
}

bool Client::Run()
{
    constexpr char semaphoreOnReadNameTemplate[] = "/tmp/client_";
    constexpr char semaphoreOnWriteNameTemplate[] = "/tmp/host_";

    syslog(LOG_INFO, "Running client");

    if (!connection_)
        return false;
    needWork_ = true;

    std::string semaphoreOnReadName = semaphoreOnReadNameTemplate + std::to_string(connection_->hostPid_);
    std::string semaphoreOnWriteName = semaphoreOnWriteNameTemplate + std::to_string(connection_->hostPid_);
    semOnRead_ = sem_open(semaphoreOnReadName.c_str(), 0);
    if (SEM_FAILED == semOnRead_)
    {
        syslog(LOG_ERR, "Create semaphore on read error");
        return false;
    }
    semOnWrite_ = sem_open(semaphoreOnWriteName.c_str(), 0);
    if (SEM_FAILED == semOnRead_)
    {
        sem_close(semOnRead_);
        syslog(LOG_ERR, "Create semaphore on read error");
        return false;
    }

    syslog(LOG_INFO, "Semaphores created");

    while (needWork_)
    {
        if (!GetGhoatState())
        {
            sem_close(semOnRead_);
            sem_close(semOnWrite_);
            return false;
        }
        if (!SendNum())
        {
            sem_close(semOnRead_);
            sem_close(semOnWrite_);
            return false;
        }
    }

    sem_close(semOnRead_);
    sem_close(semOnWrite_);

    return true;
}
