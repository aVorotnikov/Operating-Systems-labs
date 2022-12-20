#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../host/host.h"
#include "../window/ChatWin.h"
#include "../connections/abstr_conn.h"
#include "../messages/messages.h"

class Client{
private:
    std::atomic<bool> isRunning = true;
    
    // connetcions
    std::unique_ptr<AbstractConnection> conn;
    pid_t hostPid;
    pid_t clientPid;
    sem_t *hostSem;
    sem_t *clientSem;

    void connectionWork();

    bool connectionPrepare(const pid_t& hostPid);
    bool connectionReadMsgs();
    bool connectionWriteMsgs();
    void connectionClose();

    // Signals and msgs managment
    ConnectedQueue messagesIn;
    ConnectedQueue messagesOut;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);

    // Window managment
    static bool IsRun();
    static bool winRead(Message *msg);
    static void winWrite(Message msg);

    // constructions 
    Client();
    Client(const Client&) = delete;
    Client(Client&&) = delete;
public:
    static Client& getInstance();
    bool init(const pid_t& hostPid);
    void run();
    void stop();

    ~Client() = default;
};