#include "wolf.h"
#include "utils/configuration.h"
#include "goat/goat.h"

#include <sys/syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

Wolf &Wolf::getInstance()
{
    static Wolf instance;
    return instance;
}

Wolf::Wolf()
{
    conn = Connection::create();
    hostPid = getpid();
    openlog("WolfAndGoat", LOG_NDELAY | LOG_PID, LOG_USER);
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = Wolf::signalHandle;
    sigaction(SIGTERM, &sig, nullptr);
    sigaction(SIGINT, &sig, nullptr);
}

Wolf::~Wolf()
{
    kill(clientPid, SIGTERM);
    if (!conn->close())
    {
        syslog(LOG_ERR, "ERROR: failed to close connection");
    }

    if (semHost != SEM_FAILED)
    {
        sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
    }

    if (semClient != SEM_FAILED)
    {
        sem_unlink(Configuration::CLIENT_SEMAPHORE_NAME.c_str());
    }
}

bool Wolf::init(sendMessageToGuiCallback sendToGui)
{
    syslog(LOG_INFO, "Host initialization");

    sendMessageToGui = sendToGui;

    semHost = sem_open(Configuration::HOST_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (semHost == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: host semaphore not created");
        return false;
    }

    semClient = sem_open(Configuration::CLIENT_SEMAPHORE_NAME.c_str(), O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
    if (semClient == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: client semaphore not created");
        sem_unlink(Configuration::HOST_SEMAPHORE_NAME.c_str());
        return false;
    }

    pid_t childPid = fork();
    if (childPid == 0)
    {
        clientPid = getpid();

        if (Goat::getInstance().init(hostPid))
        {
            Goat::getInstance().run();
        }
        else
        {
            syslog(LOG_ERR, "ERROR: client initialization error");
            return false;
        }
        exit(EXIT_SUCCESS);
    }

    if (!conn->open(hostPid, true))
    {
        syslog(LOG_ERR, "ERROR: failed to open connection");
        return false;
    }

    Wolf::getInstance().isRun = true;
    syslog(LOG_INFO, "INFO: host initialize successfully");

    return true;
}

void Wolf::run()
{
    size_t round = 0;

    while (isRun)
    {
        if (isNewMessage)
        {
            if (!stop())
            {
                syslog(LOG_ERR, "ERROR: failed to stop wolf");
                return;
            }

            Message goatMsg;
            if (!getGoatMessage(goatMsg))
            {
                syslog(LOG_ERR, "ERROR: failed to get message from goat");
                return;
            }

            goatMsg.goatState = updateGoatState(wolfMessage.load().thrownNumber, goatMsg);
            if (!sendWolfMessage({wolfMessage.load().thrownNumber, goatMsg.goatState}))
            {
                syslog(LOG_ERR, "ERROR: send message to wolf failed");
                return;
            }
            isRun = checkRun(goatMsg.goatState);
            if (!isRun)
            {
                kill(clientPid, SIGTERM);
                clientPid = 0;
            }
            isNewMessage = false;

            sendMessageToGui("Round " + std::to_string(round));
            sendMessageToGui("Wolf number: " + std::to_string(wolfMessage.load().thrownNumber));
            sendMessageToGui("Goat number: " + std::to_string(goatMsg.thrownNumber));

            round++;
        }
    }

    kill(clientPid, SIGTERM);
}

bool Wolf::isRunning()
{
    return Wolf::getInstance().isRun;
}

void Wolf::stopRunning()
{
    Wolf::getInstance().isRun = false;
}

void Wolf::getNewWolfMessage(unsigned short thrownNum)
{
    Wolf::getInstance().isNewMessage = true;
    Wolf::getInstance().wolfMessage.store({thrownNum, GOAT_STATE::ALIVE});
}

bool Wolf::stop()
{
    if (sem_post(semClient) == -1)
    {
        syslog(LOG_ERR, "ERROR: failed to continue client");
        return false;
    }
    if (sem_wait(semHost) == -1)
    {
        syslog(LOG_ERR, "ERROR: failed to stop host");
        return false;
    }
    return true;
}

bool Wolf::getGoatMessage(Message &msg)
{
    return conn->read(msg);
}

GOAT_STATE Wolf::updateGoatState(const size_t &wolfNum, const Message &goatMessage)
{
    size_t div = std::abs(static_cast<int>(wolfNum) - static_cast<int>(goatMessage.thrownNumber));
    switch (goatMessage.goatState)
    {
    case GOAT_STATE::ALIVE:
        return div > Configuration::Goat::ALIVE_GOAT_VALIDATION ? GOAT_STATE::ALMOST_DEAD : GOAT_STATE::ALIVE;
    case GOAT_STATE::ALMOST_DEAD:
        return div > Configuration::Goat::ALMOST_DEAD_GOAT_VALIDATION ? GOAT_STATE::DEAD : GOAT_STATE::ALIVE;
    case GOAT_STATE::DEAD:
        return GOAT_STATE::DEAD;
    }

    return GOAT_STATE::DEAD;
}

bool Wolf::sendWolfMessage(const Message &msg)
{
    return conn->write(msg);
}

bool Wolf::checkRun(GOAT_STATE goatState)
{
    if (goatState == GOAT_STATE::ALIVE || goatState == GOAT_STATE::ALMOST_DEAD)
    {
        return true;
    }
    syslog(LOG_INFO, "INFO: goat dead. Game over");
    return false;
}

void Wolf::signalHandle(int sig, siginfo_t *sigInfo, void *ptr)
{
    switch (sig)
    {
    case SIGTERM:
        Wolf::getInstance().isRun = false;
        return;
    case SIGINT:
        syslog(LOG_INFO, "INFO: wolf terminate");
        exit(EXIT_SUCCESS);
        return;
    default:
        syslog(LOG_INFO, "INFO: unknown command");
    }
}