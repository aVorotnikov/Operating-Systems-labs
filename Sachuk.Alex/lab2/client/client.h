#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../host/host.h"
#include "../window/window.h"
#include "../connections/abstr_conn.h"
#include "../messages/messages.h"

class Client{
private:
    // Instance
    static Client cientInstance;
    std::atomic<bool> isRunning = true;
    
    // connetcions
    std::unique_ptr<AbstractConnection> conn;
    pid_t hostPid;
    pid_t clientPid;
    sem_t *hostSem;
    sem_t *clientSem;

    void connectionWork();

    bool connectionPrepare();
    bool connectionReadMsgs();
    bool connectionWriteMsgs();
    void connectionClose();

    // Signals and msgs managment
    ConnectedQueue messagesIn;
    ConnectedQueue messagesOut;
    static void SignalHandler(int signum, siginfo_t* info, void *ptr);

    // Window managment
    Window* window = nullptr;
    static bool IsRun();
    static bool winRead(Message *msg);
    static void winWrite(Message msg);

    // constructions 
    Client();
    Client(const Client&) = delete;
    Client(Host&&) = delete;
public:
    static Client& getInstance();
    static bool init(pid_t hostPid);
    void run();
    void stop();

    ~Client();
};