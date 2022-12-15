#include "client.h"

#include <sys/syslog.h>
#include <atomic>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include "utils/config.h"

Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

void Client::signalHandle(int sig, siginfo_t *sigInfo, void *ptr)
{
    switch (sig)
    {
    case SIGTERM:
        syslog(LOG_INFO, "INFO: client terminate");
        Client::getInstance().stop();
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO: client terminate");
        Client::getInstance().closeConnection();
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "INFO: unknown command");
    }
}

bool Client::init(pid_t hostPid, short goatId, int randomSeed)
{
    syslog(LOG_INFO, "Client initialization");
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Client::signalHandle;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
    srand(randomSeed);
    this->hostPid = hostPid;
    this->goatId = goatId;
    needToStop = false;
    throwRequested = false;
    auto baseName = std::to_string(this->hostPid) + "_" + std::to_string(this->goatId);
    auto semInName = "/wolf_n_goat_write_sem_host_" + baseName;
    auto semOutName = "/wolf_n_goat_write_sem_client_" + baseName;
    conn = Connection::createDefault(baseName, false);
    if (conn == nullptr)
    {
        syslog(LOG_ERR, "ERROR: failed to create client connection");
        closeConnection();
        return false;
    }
    if (!conn->open())
    {
        syslog(LOG_ERR, "ERROR: failed to open client connection");
        delete conn;
        conn = nullptr;
        closeConnection();
        return false;
    }
    semIn = sem_open(semInName.c_str(), 0);
    if (semIn == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to client read semaphore");
        closeConnection();
        return false;
    }
    semOut = sem_open(semOutName.c_str(), 0);
    if (semOut == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to client write semaphore");
        closeConnection();
        return false;
    }
    syslog(LOG_INFO, "Client initialized");
    return true;
}

void Client::stop()
{
    needToStop = true;
}

void Client::run()
{
    syslog(LOG_INFO, "Client start running");
    while (!needToStop)
    {
        if (!readMessage())
        {
            break;
        }
        if (!handleMessage())
        {
            break;
        }
        if (!sendMessage())
        {
            break;
        }
    }
    closeConnection();
    syslog(LOG_INFO, "Client run end");
}

void Client::closeConnection()
{
    if (semIn != SEM_FAILED)
    {
        sem_close(semIn);
    }
    if (semOut != SEM_FAILED)
    {
        sem_close(semOut);
    }
    if (conn != nullptr)
    {
        conn->close();
        delete conn;
    }
}

bool Client::readMessage()
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += Config::SEMAPHORE_TIMEOUT_SECONDS;

    if (sem_timedwait(semIn, &t) == -1)
    {
        syslog(LOG_ERR, "ERROR: Waiting timeout on client read semaphore");
        return false;
    }

    Message message;
    if (!conn->read(message))
    {
        syslog(LOG_ERR, "ERROR: Client failed to read");
        return false;
    }
    lastMessage = message;
    return true;
}

bool Client::handleMessage()
{
    switch (lastMessage.messageType)
    {
    case Message::MT_END:
        syslog(LOG_INFO, "Client got END message");
        return false;
    case Message::MT_KEEP_WAITING:
        replyMessage = lastMessage;
        replyMessage.messageType = Message::MT_KEEP_WAITING;
        break;
    case Message::MT_THROW_REQUEST:
        syslog(LOG_INFO, "Client got THROW_REQUEST message");
        replyMessage = lastMessage;
        replyMessage.messageType = Message::MT_THROW_RESPONSE;
        replyMessage.goatInfo.lastEvent = GOAT_EVENT::THROW_RECEIVED;
        replyMessage.goatInfo.thrownNumber = throwNumber();
        break;
    case Message::MT_ROUND_RESULT:
        syslog(LOG_INFO, "Client got ROUND_RESULT message, new state - %s",
               (lastMessage.goatInfo.state == GOAT_STATE::ALIVE) ? "Alive" : "Dead");
        replyMessage = lastMessage;
        replyMessage.messageType = Message::MT_KEEP_WAITING;
        break;
    default:
        syslog(LOG_ERR, "ERROR: Client got unknown message type");
        return false;
    }
    return true;
}

ushort Client::throwNumber()
{
    if (lastMessage.goatInfo.state == GOAT_STATE::DEAD)
    {
        return rand() % 50 + 1;
    }
    else
    {
        return rand() % 100 + 1;
    }
}

bool Client::sendMessage()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % Config::MAX_CLIENT_REPLY_DELAY_MILLISECONDS));
    if (!conn->write(replyMessage))
    {
        syslog(LOG_ERR, "ERROR: Client failed to send");
        return false;
    }
    if (sem_post(semOut) == -1)
    {
        syslog(LOG_ERR, "ERROR: Client write semaphore post error");
        return false;
    }
    return true;
}
