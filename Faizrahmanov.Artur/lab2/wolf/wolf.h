#pragma once

#include <unistd.h>
#include <semaphore.h>
#include <csignal>

#include "connections/conn.h"
#include <atomic>
#include <thread>
#include <functional>

class Wolf
{
private:
    using sendMessageToGuiCallback = std::function<void(const std::string&)>;

    bool isRun = false;
    sem_t *semHost{};
    sem_t *semClient{};
    pid_t hostPid = 0;
    pid_t clientPid = 0;
    std::unique_ptr<Connection> conn;

    std::atomic<Message> wolfMessage;

    bool isNewMessage = false;

public:
    static Wolf &getInstance();
    ~Wolf();

    bool init(sendMessageToGuiCallback sendToGui);
    void run();

    static bool isRunning();
    static void stopRunning();
    static void getNewWolfMessage(unsigned short thrownNum);

    sendMessageToGuiCallback sendMessageToGui;

private:
    Wolf();
    Wolf(Wolf &w) = delete;
    Wolf &operator=(const Wolf &w) = delete;

    bool stop();
    bool getGoatMessage(Message &msg);
    bool sendWolfMessage(const Message &msg);
    GOAT_STATE updateGoatState(const size_t &wolfNum, const Message& goatMessage);
    bool checkRun(GOAT_STATE goatState);
    static void signalHandle(int sig, siginfo_t *sigInfo, void *ptr);
};