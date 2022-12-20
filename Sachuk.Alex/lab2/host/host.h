#pragma once

#include <atomic>
#include <mutex>
#include <sys/types.h>
#include <semaphore.h>   
#include <bits/types/siginfo_t.h>

#include "../client/client.h"
#include "../window/ChatWin.h"
#include "../connections/abstr_conn.h"
#include "../messages/messages.h"


// 'Host' managment class: singleton
class Host {
private:
    // Instance
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
    static bool IsRun();
    static bool winRead(Message *msg);
    static void winWrite(Message msg);

    // constructions 
    Host();
    Host(const Host&) = delete;
    Host(Host&&) = delete;
public:
    static Host& getInstance();
    void run();
    void stop();

    ~Host() = default;
};
