#include "client_handler.h"

#include <sys/syslog.h>
#include <QApplication>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>
#include "shared_game_state.h"
#include "client/client.h"
#include "utils/config.h"
#include <fcntl.h>

ClientHandler::ClientHandler(short goatId, std::shared_ptr<SharedGameState> &gameState, SafeQueuePusher<Message> &&hostPusher) : hostPusher(std::move(hostPusher))
{
    hostPid = getpid();
    clientPid = -1;
    this->goatId = goatId;
    this->gameState = gameState;
    needToStop = false;
}

void ClientHandler::pushMessage(const Message::MESSAGE_TYPE &message)
{
    fromHostMessageQueue.push(message);
}

bool ClientHandler::init()
{
    syslog(LOG_INFO, "ClientHandler initialization");
    auto baseName = std::to_string(this->hostPid) + "_" + std::to_string(this->goatId);
    semInName = "/wolf_n_goat_write_sem_client_" + baseName;
    semOutName = "/wolf_n_goat_write_sem_host_" + baseName;
    semIn = sem_open(semInName.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (semIn == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to host read semaphore");
        closeConnection();
        return false;
    }
    semOut = sem_open(semOutName.c_str(), O_CREAT | O_EXCL, 0777, 0);
    if (semOut == SEM_FAILED)
    {
        syslog(LOG_ERR, "ERROR: failed to connect to host write semaphore");
        closeConnection();
        return false;
    }
    conn = Connection::createDefault(baseName, true);
    if (conn == nullptr)
    {
        syslog(LOG_ERR, "ERROR: failed to create host connection");
        closeConnection();
        return false;
    }
    auto seed = rand();
    clientPid = fork();
    if (clientPid == 0)
    {
        {
            if (!Client::getInstance().init(hostPid, goatId, seed))
            {
                syslog(LOG_ERR, "ERROR: failed to init Client");
                exit(EXIT_FAILURE);
            }
            Client::getInstance().run();
        }
        exit(EXIT_SUCCESS);
    }
    if (!conn->open())
    {
        syslog(LOG_ERR, "ERROR: failed to open host connection");
        delete conn;
        conn = nullptr;
        closeConnection();
        return false;
    }
    syslog(LOG_INFO, "ClientHandler initialized");
    return true;
}

void ClientHandler::closeConnection()
{
    if (semIn != SEM_FAILED)
    {
        sem_close(semIn);
        sem_unlink(semInName.c_str());
    }
    if (semOut != SEM_FAILED)
    {
        sem_close(semOut);
        sem_unlink(semOutName.c_str());
    }
    if (conn != nullptr)
    {
        conn->close();
        delete conn;
    }
}

void ClientHandler::run()
{
    syslog(LOG_INFO, "ClientHandler start running");
    while (!needToStop)
    {
        state = Message::MT_KEEP_WAITING;
        if (fromHostMessageQueue.size() == 0)
        {
            fromHostMessageQueue.waitForMessage(Config::PING_MAX_INTERVAL);
        }
        if (!fromHostMessageCheck())
        {
            break;
        }
        if (!sendClientMessage())
        {
            break;
        }
        if (!fromClientMessageCheck())
        {
            break;
        }
    }
    syslog(LOG_INFO, "ClientHandler waiting for client process to end");
    waitpid(clientPid, 0, 0);
    closeConnection();
    GoatInfo goatInfo;
    gameState->getGoatInfo(goatId, goatInfo);
    hostPusher.push(Message(Message::MT_END, goatInfo));
    syslog(LOG_INFO, "ClientHandler run end");
}

bool ClientHandler::fromHostMessageCheck()
{
    while (fromHostMessageQueue.size() != 0)
    {
        Message::MESSAGE_TYPE message = fromHostMessageQueue.front();
        fromHostMessageQueue.pop();
        switch (message)
        {
        case Message::MT_END:
            syslog(LOG_INFO, "ClientHandler got END message from Host");
            needToStop = true;
            state = Message::MT_END;
            sendClientMessage();
            return false;
        case Message::MT_ROUND_RESULT:
            state = Message::MT_ROUND_RESULT;
            break;
        case Message::MT_THROW_REQUEST:
            state = Message::MT_THROW_REQUEST;
            break;
        default:
            syslog(LOG_ERR, "ERROR: ClientHandler got unknown message type from Host");
            needToStop = true;
            state = Message::MT_END;
            sendClientMessage();
            return false;
        }
    }
    return true;
}

bool ClientHandler::sendClientMessage()
{
    Message message;
    gameState->getGoatInfo(goatId, message.goatInfo);
    message.messageType = state;
    if (!conn->write(message))
    {
        syslog(LOG_ERR, "ERROR: Host failed to send");
        return false;
    }
    if (sem_post(semOut) == -1)
    {
        syslog(LOG_ERR, "ERROR: Host write semaphore post error");
        return false;
    }
    return true;
}

bool ClientHandler::fromClientMessageCheck()
{
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += Config::SEMAPHORE_TIMEOUT_SECONDS;

    if (sem_timedwait(semIn, &t) == -1)
    {
        syslog(LOG_ERR, "ERROR: Waiting timeout on host read semaphore");
        return false;
    }

    Message message;
    if (!conn->read(message))
    {
        syslog(LOG_ERR, "ERROR: Host failed to read");
        return false;
    }
    if (message.messageType != Message::MT_KEEP_WAITING)
    {
        hostPusher.push(message);
    }
    if (message.messageType == Message::MT_END)
    {
        syslog(LOG_INFO, "ClientHandler got END message from client");
        needToStop = true;
        return false;
    }
    return true;
}