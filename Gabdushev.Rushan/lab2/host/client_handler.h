#pragma once
#include <atomic>
#include <memory>
#include <semaphore.h>
#include "utils/message.h"
#include "utils/safe_queue.h"
#include "connections/conn.h"


class SharedGameState;

class ClientHandler
{
private:
    std::shared_ptr<SharedGameState> gameState;
    SafeQueuePusher<Message> hostPusher;
    SafeQueue<Message::MESSAGE_TYPE> fromHostMessageQueue;
    std::atomic_bool needToStop;
    Message::MESSAGE_TYPE state;
    Connection *conn = nullptr;
    sem_t *semIn = SEM_FAILED;
    sem_t *semOut = SEM_FAILED;
    std::string semInName, semOutName;
    pid_t hostPid;
    pid_t clientPid;

public:
    short goatId;
    ClientHandler(short goatId, std::shared_ptr<SharedGameState> &gameState, SafeQueuePusher<Message> &&hostPusher);
    void pushMessage(const Message::MESSAGE_TYPE &message);
    bool init();
    void run();

private:
    void closeConnection();
    bool fromHostMessageCheck();
    bool sendClientMessage();
    bool fromClientMessageCheck();
};