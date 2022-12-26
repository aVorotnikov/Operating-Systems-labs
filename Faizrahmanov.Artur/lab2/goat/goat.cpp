#include "goat.h"
#include "utils/configuration.h"

#include <csignal>
#include <sys/syslog.h>
#include <time.h>
#include <random>
#include <iostream>
#include <cstring>

Goat &Goat::getInstance()
{
    static Goat instance;
    return instance;
}

Goat::Goat()
{
    conn = Connection::create();

    signal(SIGTERM, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGINT, signalHandler);
}

Goat::~Goat()
{
    if (clientSemaphore != SEM_FAILED)
    {
        sem_close(clientSemaphore);
    }
    if (hostSemaphore != SEM_FAILED)
    {
        sem_close(hostSemaphore);
    }
    if (!conn->close())
    {
        syslog(LOG_ERR, "ERROR: failed close connection");
    }
    kill(hostPid, SIGTERM);
}

bool Goat::init(const pid_t &hostPid)
{
    syslog(LOG_INFO, "Client initialization");

    this->hostPid = hostPid;

    if (!conn->open(hostPid, false))
    {
        syslog(LOG_ERR, "ERROR: failed to open client connection");
        return false;
    }

    if ((hostSemaphore = sem_open(Configuration::HOST_SEMAPHORE_NAME.c_str(), 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to host semaphore");
        return false;
    }

    if ((clientSemaphore = sem_open(Configuration::CLIENT_SEMAPHORE_NAME.c_str(), 0)) == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to client semaphore");
        return false;
    }

    isRun = true;

    syslog(LOG_INFO, "Client initialized");

    return true;
}

void Goat::run()
{
    syslog(LOG_INFO, "Client start running");

    unsigned short goatNumber = getGoatNumber(GOAT_STATE::ALIVE);
    if (!sendGoatMessage({goatNumber, GOAT_STATE::ALIVE}))
    {
        syslog(LOG_ERR, "ERROR: failed to send goat message");
        return;
    }

    while (isRun)
    {
        if (!stopClient())
        {
            syslog(LOG_ERR, "ERROR: failed to stop client");
            return;
        }
        Message wolfMessage;
        if (!getWolfMessage(wolfMessage))
        {
            syslog(LOG_ERR, "ERROR: failed to read wolf message");
            return;
        }
        goatNumber = getGoatNumber(wolfMessage.goatState);
        if (!sendGoatMessage({goatNumber, wolfMessage.goatState}))
        {
            syslog(LOG_ERR, "ERROR: failed to send goat message");
            return;
        }
        isRun = (wolfMessage.goatState != GOAT_STATE::DEAD);
    }

    syslog(LOG_INFO, "Client run end");
}

bool Goat::getWolfMessage(Message &msg)
{
    return conn->read(msg);
}

size_t Goat::getGoatNumber(GOAT_STATE state)
{
    using namespace Configuration::Goat;

    std::random_device seeder;
    std::mt19937 rng(seeder());
    std::uniform_int_distribution<int> genAliveNum(MIN_NUMBER, MAX_ALIVE_NUMBER);
    std::uniform_int_distribution<int> genAlmostDeadNum(MIN_NUMBER, MAX_ALMOST_DEATH_NUMBER);

    size_t res = 0;

    switch (state)
    {
    case GOAT_STATE::ALIVE:
        res = genAliveNum(rng);
        break;
    case GOAT_STATE::ALMOST_DEAD:
        res = genAlmostDeadNum(rng);
        break;
    case GOAT_STATE::DEAD:
        break;
    }

    return res;
}

bool Goat::sendGoatMessage(const Message &msg)
{
    return conn->write(msg);
}

bool Goat::stopClient()
{
    if (sem_post(hostSemaphore) == -1)
    {
        syslog(LOG_ERR, "ERROR: host semaphore can't continue");
        return false;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += Configuration::TIME_OUT;
    if (sem_timedwait(clientSemaphore, &ts) == -1)
    {
        std::cout << strerror(errno) << std::endl;
        syslog(LOG_ERR, "ERROR: client semaphore can't wait");
        return false;
    }
    return true;
}

void Goat::signalHandler(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        Goat::getInstance().isRun = false;
        exit(EXIT_SUCCESS);
    case SIGINT:
        syslog(LOG_INFO, "INFO: receive delete client request");
        exit(EXIT_SUCCESS);
    case SIGUSR1:
        syslog(LOG_INFO, "INFO: client end game");
        kill(Goat::getInstance().hostPid, SIGTERM);
        exit(EXIT_SUCCESS);
    default:
        syslog(LOG_INFO, "INFO: unknow command");
    }
}
